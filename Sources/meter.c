#include "meter.h"
#include "OS.h"
#include "analog.h"
#include "MyPacket.h"
#include "Cpu.h"
#include "Protocol.h"
#include "PIT.h"
#include "Math.h"
#include "Tariff.h"
#include "SampleQueue.h"

#define SAMPLE_PERIOD_BASE 1187350 // 52.5 Hz
#define NB_ANALOG_CHANNELS 2

#define VOLTAGE_CHANNEL 1
#define CURRENT_CHANNEL 2

#define VOLTAGE_THREAD 0
#define CURRENT_THREAD 1

#define THREAD_STACK_SIZE 300

/*! @brief Data structure used to pass Analog configuration to a user thread
 *
 */
typedef struct AnalogThreadData
{
  OS_ECB* semaphore;
  OS_ECB* signal;   // Semaphore to be signaled
  uint8_t channelNb;
  uint16_t* RMS;
  uint8_t  ratio;   // ratio from output to ADC to raw input
  SampleQueue queue;
} TAnalogThreadData;

OS_THREAD_STACK(CalcThreadStack, THREAD_STACK_SIZE); /*!< The stack for the Calc thread. */
static uint32_t AnalogThreadStacks[NB_ANALOG_CHANNELS][THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));

uint32_t TickPeriod = SAMPLE_PERIOD_BASE;
uint8_t FrequencyDiff = 25;

int16_t Meter_Voltage;       /*!< In 16Q8 format */
int16_t Meter_Current;

uint16_t Meter_VoltageRMS;   /*!< In 16Q8 format */
uint16_t Meter_CurrentRMS;

int16_t InstantaneousVoltage;/*!< In 16Q8 format */
int16_t InstantaneousCurrent;

uint32_t Meter_AveragePower;  /*!< Unit: kW.     P=VIcos, calculated every sample time        */
uint64_t Meter_Energy;        /*!< Unit: Joule.  E=sum(p)*Ts, calculated every period         */
uint64_t Meter_Cost;          /*!< Unit: Cent.   Cost of electricity, calculated every period */
uint16_t Meter_PowerFactor;   /*!< 16Q8, from 0 to 1                                        */
uint8_t Phase;
// ----------------------------------------
// Thread priorities
// 0 = highest priority
// ----------------------------------------
const uint8_t ANALOG_THREAD_PRIORITIES[NB_ANALOG_CHANNELS] = {3, 4};

/*! @brief Analog thread configuration data
 *
 */
static TAnalogThreadData AnalogThreadData[NB_ANALOG_CHANNELS] =
{
  {
    .semaphore = NULL,
    .channelNb = VOLTAGE_CHANNEL,
    .RMS = &Meter_VoltageRMS,
    .ratio = 100,
    .signal = NULL
  },
  {
    .semaphore = NULL,
    .channelNb = CURRENT_CHANNEL,
    .RMS = &Meter_CurrentRMS,
    .ratio = 1,
    .signal = NULL
  }
};

/*! @brief Semaphore for calculation thread
 *
 */
static OS_ECB* CalcSemaphore;


/*! @brief Get the frequency difference between current voltage frequency and nominal frequency(50 Hz)
 *
 *  @return 0-50
 */
uint8_t Meter_GetFrequencyDiff()
{
  return FrequencyDiff;
}

/*! @brief Measure the frequency of voltage's sine wave
 *
 *  @param analogInputValue The latest voltage value
 */
void MeterFrequency(int16_t analogInputValue)
{
  static bool started = false; // Start from the first zero crossing
  static int16_t previousVoltage = 0;
  static uint8_t tickCnt = 0;

  if (previousVoltage * analogInputValue < 0 && started)
  {
    // half a cycle
    if (tickCnt > 7)
    // Sample frequency too fast
    {
      TickPeriod = SAMPLE_PERIOD_BASE + SAMPLE_TIME_INCREMENT*(++FrequencyDiff);
      PIT_Enable(false);
      PIT_Set(TickPeriod, true);
      started = false;
      previousVoltage = 0;
    }
    else if (tickCnt < 7)
    // Sample frequency too slow
    {
      TickPeriod = SAMPLE_PERIOD_BASE + SAMPLE_TIME_INCREMENT*(--FrequencyDiff);
      PIT_Enable(false);
      PIT_Set(TickPeriod, true);
      started = false;
      previousVoltage = 0;
    }
    tickCnt = 0;
  }
  else if (!started)
  // Found the first zero crossing place
  {
    if (previousVoltage * analogInputValue < 0)
      started = true;
  }
  else
  {
    tickCnt ++;
  }
  previousVoltage = analogInputValue;
}

/*! @brief Process latest current and voltage value
 *
 *  @param pData Thread data for voltage thread and current thread.
 */
void AnalogLoopbackThread(void* pData)
{
  // Make the code easier to read by giving a name to the typecast'ed pointer
  #define analogData ((TAnalogThreadData*)pData)

  for (;;)
  {
    int16_t analogInputValue;

    (void)OS_SemaphoreWait(analogData->semaphore, 0);

    if (analogData->channelNb == 1)
    {
      analogInputValue = Meter_Voltage;
      InstantaneousVoltage = Meter_Voltage;
      MeterFrequency(analogInputValue);
    }
    else
    {
      analogInputValue = Meter_Current;
      InstantaneousCurrent = Meter_Current;
    }

    // Convert to 32Q16 format
    uint32_t convertedValue = analogInputValue*2*10;
    analogData->queue.latestValue = convertedValue;

    // 32Q16 * 32Q16 = 64Q32
    // Maximum value of sample is 655360
    // 655360 * 655360 = 0x 0064 0000 0000
    // So its square will not exceed 48th bit. It's safe to discard the highest 16 bits
    uint32_t squared = ((uint64_t)((analogInputValue*2*10)*(uint64_t)(analogInputValue*2*10)) >> 16);
    SQ_Put(&(analogData->queue), squared);

    // First time, Wait until 16 samples have been retrieved
    if (*analogData->RMS == 0 && analogData->queue.nb == 16 && analogData->queue.firstTime)
    {
      // Calculate RMS for the first time
      *analogData->RMS = Math_SquareRoot(0, (analogData->queue.sum) >> 4, 0) * analogData->ratio;
      // In case the actual RMS value is 0
      analogData->queue.firstTime = false;
    }
    // Update every time since the first time
    else if (!analogData->queue.firstTime)
    {
      // Since Voltage RMS is no more than 250V, no overflow
      *analogData->RMS = (Math_SquareRoot(*analogData->RMS/(analogData->ratio), (analogData->queue.sum) >> 4, 1)) * analogData->ratio;
    }

    if (analogData->signal)
      OS_SemaphoreSignal(analogData->signal);
  }
}

/*! @brief The thread will be executed every sample time.
 *
 */
void CalcThread(void* pData)
{
  for (;;)
  {
    (void)OS_SemaphoreWait(CalcSemaphore, 0);

    static uint8_t cnt = 0;
    // Calculate energy, cost ..
    int32_t instantaneousPower  = 0;
    static int32_t sumOfPower   = 0;
    uint32_t energyForOnePeriod = 0;

    // InstantaneousCurrent and InstantaneousVoltage are of base 10/32768
    // Their product is of base 100/(2^30)
    instantaneousPower = InstantaneousCurrent * InstantaneousVoltage;

    // According to the input range, int32_t is large enough to hold all the products
    sumOfPower += instantaneousPower;
    cnt ++;

    if (cnt == SAMPLES_PER_CYCLE)
    {
      uint64_t energyForOnePeriod;
      cnt = 0; // reset counter
      // Convert from base 100/(2^30) to 32Q16(1/2^16)
      // 100/(2^30) * (2^16) = 25/4096 = 1/164
      sumOfPower = (sumOfPower + 82) / 164;

      // Handle situations when Voltage's frequency and Current's frequency don't match
      if (sumOfPower < 0)
      {
        sumOfPower = 0;
      }
      // 32Q16 * 32Q16 = 64Q32, 100 is the ratio of raw to output
      energyForOnePeriod = (uint64_t)sumOfPower * (uint64_t)Protocol_GetTime(100) ;
      Meter_Energy += energyForOnePeriod;

      Meter_AveragePower = sumOfPower * 100 / 16 / 1000;

      // 64Q32 / 32Q16 = 64Q16
      // convert from 64Q16 to 16Q8
      Meter_PowerFactor = (uint16_t)(((((uint64_t)Meter_VoltageRMS*(uint64_t)Meter_CurrentRMS) << 16)/(1000 * Meter_AveragePower)) >> 8);

      // Convert energy from Joule to kWh and 64Q32 to 32Q16
      uint64_t cost = 0;;
      uint64_t rate = Tariff_GetRate();
      cost = (energyForOnePeriod >> 16) ;
      cost *= rate;
      cost /= 3600000;

      Meter_Cost += cost;
      sumOfPower = 0;
    }

  }
}

/*! @brief Call back function of Meter module.
 *
 *  @note This will be called by PIT
 */
void MeterCallback()
{
  OS_SemaphoreSignal(AnalogThreadData[VOLTAGE_THREAD].semaphore);
  OS_SemaphoreSignal(AnalogThreadData[CURRENT_THREAD].semaphore);
}

/*! @brief Initialize meter module by creating threads and enabling timer.
 *  @param moduleClk The module clock rate in Hz.
 */
bool Meter_Init(const uint32_t moduleClk)
{
  OS_ERROR error;

  Meter_VoltageRMS = 0;
  Meter_CurrentRMS = 0;
  Meter_Energy = 0;
  Meter_Cost   = 0;
  Phase  = 0;

  // Generate the global analog semaphores
  for (uint8_t analogNb = 0; analogNb < NB_ANALOG_CHANNELS; analogNb++)
  {
    AnalogThreadData[analogNb].semaphore = OS_SemaphoreCreate(0);
    SQ_Init(&AnalogThreadData[analogNb].queue);
  }

  CalcSemaphore = OS_SemaphoreCreate(0);
  AnalogThreadData[CURRENT_THREAD].signal = CalcSemaphore;

  error |= OS_ThreadCreate(CalcThread,
                           NULL,
                           &CalcThreadStack[THREAD_STACK_SIZE - 1],
                           8);

  for (uint8_t threadNb = 0; threadNb < NB_ANALOG_CHANNELS; threadNb ++)
  {
    error |= OS_ThreadCreate(AnalogLoopbackThread,
                             &AnalogThreadData[threadNb],
                             &AnalogThreadStacks[threadNb][THREAD_STACK_SIZE - 1],
                             ANALOG_THREAD_PRIORITIES[threadNb]);
  }

  if (PIT_Init(moduleClk, MeterCallback, NULL))
  {
    PIT_Set(TickPeriod, true);
  }
  return !error;
}
