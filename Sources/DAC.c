#include "DAC.h"
#include "OS.h"
#include "Cpu.h"
#include "analog.h"

#define THREAD_STACK_SIZE 300

OS_THREAD_STACK(OutputThreadStack, THREAD_STACK_SIZE); /*!< The stack for the Output thread. */

static int16_t VoltageSineWave[64];
static int16_t CurrentSineWave[64];

// Base: 10/32768
// Minimum Voltage and Current
static int16_t const MinVoltage = 9266;    // represents 282.8V which is actually 2.82V
static int16_t const MaxVoltage = 11583;   // 3.53  V
static int16_t const MinCurrent = 0;       // 0     A
static int16_t const MaxCurrent = 23170;   // 7.072 A
static uint8_t const MinPhase   = 48;       // -90  degree

// Step size
static int16_t const VoltageStepSize = 1;   // Every step represents 0.03052 mV
static int16_t const CurrentStepSize = 1;   // Every step represents 305.2 uA
static uint8_t const PhaseStepSize   = 1;   // Every step represents 5.625 degree

static int16_t VoltageAmp;
static int16_t CurrentAmp;

static uint8_t Phase;

bool DAC_TestMode = false;

OS_ECB* OutputSemaphore;

static uint8_t NbV = 0;
static uint8_t NbC = 0;

// 32Q16, range from 0 to 1, represents different parts of the sine wave
const int32_t Ratio[64] = {0, 6423, 12785, 19024, 25079, 30893, 36409, 41575, 46340, 50660, 54491, 57797,
                          60547, 62714, 64276, 65220, 65536, 65220, 64276, 62714, 60547, 57797, 54491,
                          50660, 46340, 41575, 36409, 30893, 25079, 19024, 12785, 6423, 0, -6423, -12785,
                          -19024, -25079, -30893, -36409, -41575, -46340, -50660, -54491, -57797, -60547,
                          -62714, -64276, -65220, -65536, -65220, -64276, -62714, -60547, -57797, -54491,
                          -50660, -46340, -41575, -36409, -30893, -25079, -19024, -12785, -6423};


/*! @brief Updates the array for sine wave with give amplitude
 *
 *  @param sineWave pointer to the first number in array
 *  @param amp Amplitude of the sine wave
 */
void UpdateSineWave(int16_t* sineWave, int16_t amp)
{
  uint8_t i;
  for (i = 0; i < 64; i ++)
    // Base: 1/65536 * 10/32768 = 10/(2^31)
    // Convert to 10/32768: *65536
    sineWave[i] = (int16_t)((int64_t)Ratio[i] * (int64_t)amp >> 16);
}

/*! @brief Callback function that signals the output semaphore
 *
 */
void DAC_Callback()
{
  OS_SemaphoreSignal(OutputSemaphore);
}

/*! @brief Thread to output to channels
 *
 *  @param pData Thread data(not used)
 */
void OutputThread(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(OutputSemaphore, 0);
    OS_DisableInterrupts();
    Analog_Put(1, VoltageSineWave[NbV]);
    Analog_Put(2, CurrentSineWave[NbC]);
    OS_EnableInterrupts();
    NbV = (NbV + 4) % 64;
    NbC = (NbC + 4) % 64;
  }
}

/*! @brief Set up DAC before first use
 *
 */
bool DAC_Init()
{
  VoltageAmp = MinVoltage;
  CurrentAmp = MinCurrent;

  // Update voltage and current sine wave with minimum amplitude
  UpdateSineWave(VoltageSineWave, VoltageAmp);
  UpdateSineWave(CurrentSineWave, CurrentAmp);

  OutputSemaphore = OS_SemaphoreCreate(0);

  OS_ThreadCreate(OutputThread,
                  NULL,
                  &OutputThreadStack[THREAD_STACK_SIZE - 1],
                  2);
}

/*! @brief Set Voltage Amplitude for DAC
 *
 *  @param steps steps away from minimum voltage amplitude
 */
void DAC_SetVoltageAmp(uint16_t steps)
{
  // Validity of Steps has been checked so set steps directly
  VoltageAmp = MinVoltage + steps*VoltageStepSize;
  UpdateSineWave(VoltageSineWave, VoltageAmp);
}

/*! @brief Set Current Amplitude for DAC
 *
 *  @param steps steps away from minimum Current amplitude
 */
void DAC_SetCurrentAmp(uint16_t steps)
{
  // Validity of Steps has been checked so set steps directly
  CurrentAmp = MinCurrent + steps*CurrentStepSize;
  UpdateSineWave(CurrentSineWave, CurrentAmp);
}

/*! @brief Set Phase for DAC
 *
 *  @param steps away from minimum phase
 */
void DAC_SetPhase(uint8_t steps)
{
  Phase = (MinPhase + PhaseStepSize * steps) % 64;
  NbC = (NbV + Phase) % 64;
}

/*! @brief Start DAC, basically set boolean to true
 *
 */
void DAC_Start()
{
  Phase = MinPhase;
  DAC_TestMode = true;
}

/*! @brief Stop DAC, basically set boolean to false
 *
 */
void DAC_Stop()
{
  DAC_TestMode = false;
}

/*! @brief Get mode of DAC
 *
 *  @return uint8_t 1 running
 */
uint8_t DAC_GetMode()
{
  return (uint8_t)DAC_TestMode;
}
