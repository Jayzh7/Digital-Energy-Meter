#include "Interface.h"
#include "FTM.h"
#include "Cpu.h"
#include "OS.h"
#include "Debounce.h"
#include "Display.h"
#include "DAC.h"
#include "meter.h"

#define THREAD_STACK_SIZE 300

typedef enum
{
  DORMANT,
  METERING_TIME,
  AVERAGE_POWER,
  TOTAL_COST,
  TOTAL_ENERGY
}TDisplayStatus;

void DisplayCallback();

/*! @brief Display FTM channel configuration data
 *
 */
static TFTMChannel DisplayChannel = {
    .channelNb = 0,
    .delayCount = 24414,
    .timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE,
    .ioType.outputAction = TIMER_OUTPUT_LOW,
    .ioType.inputDetection = TIMER_INPUT_OFF,
    .userArguments = NULL,
    .userFunction = DisplayCallback
};

static TDisplayStatus DisplayStatus;
static uint8_t DisplayCnt;
static OS_ECB* DisplaySemaphore;      /*!< Semaphore for display thread. */
static OS_ECB* PushButtonSemaphore;

OS_THREAD_STACK(PushButtonThreadStack, THREAD_STACK_SIZE); /*!< The stack for the Push Button thread. */
OS_THREAD_STACK(DisplayThreadStack, THREAD_STACK_SIZE);    /*!< The stack for the Display thread. */

/*! @brief Callbcak of FTM, increment counter and determine if it reaches end of the period
 *
 */
void DisplayCallback()
{
  if (DisplayCnt == 15)
  {
    // Reaches the end of the display period, go back to dormant
    DisplayCnt = 0;
    DisplayStatus = DORMANT;
  }
  else
  {
    // Display current chosen status
    DisplayCnt ++;
    OS_SemaphoreSignal(DisplaySemaphore);
    FTM_StartTimer(&DisplayChannel);
  }
}

/*! @brief Display data according to current state
 *
 *  @param pData thread data
 */
void DisplayThread(void* pData)
{
  for (;;)
  {
    OS_SemaphoreWait(DisplaySemaphore, 0);

    switch (DisplayStatus)
    {
      case DORMANT:
        // Do nothing
        break;
      case METERING_TIME:
        Display_MeteringTime();
        break;
      case AVERAGE_POWER:
        Display_AveragePower(0);
        break;
      case TOTAL_ENERGY:
        Display_TotalEnergy(0);
        break;
      case TOTAL_COST:
        Display_TotalCost(0);
        break;
    }
  }
}

/*! @biref Call back function of Push button
 *
 *  @note This function will be called if SW1 is pressed
 */
void PushButtonCallback()
{
  OS_SemaphoreSignal(PushButtonSemaphore);
}

/*! @brief Thread to execute task upon push button is pressed
 *
 *  @param pData thread data
 */
void PushButtonThread(void* pData)
{
  for (;;)
  {
     OS_SemaphoreWait(PushButtonSemaphore, 0);

//     OS_DisableInterrupts();
     switch (DisplayStatus)
     {
       case DORMANT:
         DisplayStatus = METERING_TIME;
         break;
       case METERING_TIME:
         DisplayStatus = AVERAGE_POWER;
         break;
       case AVERAGE_POWER:
         DisplayStatus = TOTAL_ENERGY;
         break;
       case TOTAL_ENERGY:
         DisplayStatus = TOTAL_COST;
         break;
       case TOTAL_COST:
         DisplayStatus = METERING_TIME;
         break;
     }
//     OS_EnableInterrupts();
     DisplayCnt = 0;
     FTM_StartTimer(&DisplayChannel);
  }
}

/*! @brief Initialize the Interface module before first use
 *         Create threads and initialize global variables
 */
bool Interface_Init()
{
  OS_ERROR error;

  Debounce_Init(DisplayChannel, PushButtonCallback, NULL);
  Display_Init();
  PushButtonSemaphore = OS_SemaphoreCreate(0);
  DisplaySemaphore = OS_SemaphoreCreate(0);

  DisplayStatus = DORMANT;
  DisplayCnt = 0;

  error = OS_ThreadCreate(PushButtonThread,
                          NULL,
                          &PushButtonThreadStack[THREAD_STACK_SIZE - 1],
                          6);
  error = OS_ThreadCreate(DisplayThread,
                          NULL,
                          &DisplayThreadStack[THREAD_STACK_SIZE - 1],
                          9);
}

