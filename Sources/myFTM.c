/*! @file FTM.c
 *
 *  @brief Routines for setting up the FlexTimer module (FTM) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the FlexTimer module (FTM).
 *
 *  @author Zhengjie Huang
 *  @date 2017-09-05
 */

#include "FTM.h"
#include "Cpu.h"
#include "LEDs.h"
#include "OS.h"
#include "packet.h"

#define THREAD_STACK_SIZE 100
#define NB_OF_CHANNELS 8

OS_THREAD_STACK(FTMThreadStack, THREAD_STACK_SIZE); /*!< The stack for the FTM thread. */

typedef struct
{
  void (*callbackFunction)(void *);
  void *arguments;
}TFTMCallback;

static TFTMCallback FTM0Callback[NB_OF_CHANNELS];

/*! @brief Sets up the FTM before first use.
 *
 *  Enables the FTM as a free running 16-bit counter.
 *  @return bool - TRUE if the FTM was successfully initialized.
 */
bool FTM_Init(){
  // enable FTM0 clock
  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

  FTM0_CNTIN = 0x0;
  FTM0_MOD = 0xffff;
  FTM0_CNT = 0x0;

  // Disable TOF interrupts. Use software polling.
  FTM0_SC &= ~FTM_SC_TOIE_MASK;
  // Counter operates in Up Counting mode
  FTM0_SC &= ~FTM_SC_CPWMS_MASK;
  // use fixed frequency clock (24.4140625kHz)
  FTM0_SC |= (FTM_SC_CLKS(2) | FTM_SC_PS(0));

  // Free running counter
  FTM0_MODE |= FTM_MODE_FTMEN_MASK;

  // Initialize NVIC
  // Vector 78, IRQ=62
  // Clear any pending interrupts on FTM
  NVICICPR1 = (1 << 30);
  // Enable interrupts from FTM module
  NVICISER1 = (1 << 30);

  return true;
}

/*! @brief Sets up a timer channel.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *    channelNb is the channel number of the FTM to use.
 *    delayCount is the delay count (in module clock periods) for an output compare event.
 *    timerFunction is used to set the timer up as either an input capture or an output compare.
 *    ioType is a union that depends on the setting of the channel as input capture or output compare:
 *      outputAction is the action to take on a successful output compare.
 *      inputDetection is the type of input capture detection.
 *    userFunction is a pointer to a user callback function.
 *    userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the timer was set up successfully.
 *  @note Assumes the FTM has been initialized.
 */
bool FTM_Set(const TFTMChannel* const aFTMChannel)
{
  if (aFTMChannel->ioType.outputAction != TIMER_FUNCTION_OUTPUT_COMPARE)
    return false;

  if (aFTMChannel->channelNb >= NB_OF_CHANNELS)
    return false;

  // configure toggle output on match
  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSA_MASK;
  FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSB_MASK;
  FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK;
  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK;

  FTM0Callback[aFTMChannel->channelNb].callbackFunction = aFTMChannel->userFunction;
  FTM0Callback[aFTMChannel->channelNb].arguments = aFTMChannel->userArguments;

  return true;
}


/*! @brief Starts a timer if set up for output compare.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *  @return bool - TRUE if the timer was started successfully.
 *  @note Assumes the FTM has been initialized.
 */
bool FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{

  // Set for timer
  FTM0_CnV(aFTMChannel->channelNb) = FTM0_CNTIN + aFTMChannel->delayCount;
  FTM0_CNT = 0;

  if (FTM0_CnSC(aFTMChannel->channelNb) & FTM_CnSC_CHF_MASK)
  {
    FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHF_MASK;
  }
  // Enable FTM module
  FTM0_MODE |= FTM_MODE_FTMEN_MASK;
  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_CHIE_MASK;
}

/*! @brief Interrupt service routine for the FTM.
 *
 *  If a timer channel was set up as output compare, then the user callback function will be called.
 *  @note Assumes the FTM has been initialized.
 */
void __attribute__ ((interrupt)) FTM0_ISR(void)
{
  int i;
  OS_ISREnter();

  for (i = 0; i < NB_OF_CHANNELS; i ++)
  {
    if ((FTM0_CnSC(i) & FTM_CnSC_CHF_MASK) && (FTM0_CnSC(i) & FTM_CnSC_CHIE_MASK))
    {
      FTM0_CnSC(i) &= ~FTM_CnSC_CHF_MASK;  // Clear Interrupt flag

      FTM0Callback[i].callbackFunction(FTM0Callback[i].arguments);
      //      FTM0_CnSC(i) &= ~FTM_CnSC_CHIE_MASK; // Disable interrupt
    }
  }

  OS_ISRExit();
}
