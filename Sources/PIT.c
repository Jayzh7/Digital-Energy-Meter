/*! @file
 *
 *  @brief Routines for controlling Periodic Interrupt Timer (PIT) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the periodic interrupt timer (PIT).
 *
 *  @author Zhengjie Huang
 *  @date 2017-09-05
 */

#include "PIT.h"
#include "Cpu.h"
#include "LEDs.h"
#include "meter.h"
#include "analog.h"
#include "OS.h"
#include "DAC.h"

#define NANO_SECONDS_IN_A_SECOND 1000000000
#define NANO_SECONDS_IN_10_MS 10000000

#define VOLT_CHANNEL 1
#define CURR_CHANNEL 2

uint32_t ModuleClk;

bool DAC_TestMode;

void (*PITCallback)(void*);
void *PITArguments;

int16_t Meter_Voltage;
int16_t Meter_Current;

/*! @brief Sets up the PIT before first use.
 *
 *  Enables the PIT and freezes the timer when debugging.
 *  @param moduleClk The module clock rate in Hz.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the PIT was successfully initialized.
 *  @note Assumes that moduleClk has a period which can be expressed as an integral number of nanoseconds.
 */
bool PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments){

  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

  PITCallback = userFunction;
  PITArguments = userArguments;
  ModuleClk = moduleClk;

  PIT_MCR |= PIT_MCR_MDIS_MASK;    // Disable PIT module
  PIT_MCR |= PIT_MCR_FRZ_MASK;      // Freeze time when debugging
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;  // Clear Timer interrupt flag

 // for(int i = 0; i < moduleClk+5; i ++) ;
  // Initialize NVIC
  // Vector 84, IRQ=68
  // NVIC non-IPR=2 IPR=17
  // Clear any pending interrupts on PIT
  NVICICPR2 = (1 << 4);
  // Enable interrupts from PIT module
  NVICISER2 = (1 << 4);

  PIT_MCR = 0x00;
  return true;
}

/*! @brief Sets the value of the desired period of the PIT.
 *
 *  @param period The desired value of the timer period in nanoseconds.
 *  @param restart TRUE if the PIT is disabled, a new value set, and then enabled.
 *                 FALSE if the PIT will use the new value after a trigger event.
 *  @note The function will enable the timer and interrupts for the PIT.
 */
void PIT_Set(const uint32_t period, const bool restart){
  if (restart)
  {
    // set a new value and enable
    PIT_Enable(false);
    PIT_LDVAL0 = (period/(NANO_SECONDS_IN_A_SECOND / ModuleClk)) - 1;
    PIT_Enable(true);
    PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;      // start Timer 0
  }
  else
  { // use the new value after a trigger event
    PIT_LDVAL0 = (period/(NANO_SECONDS_IN_A_SECOND / ModuleClk)) - 1;
  }
}

/*! @brief Enables or disables the PIT.
 *
 *  @param enable - TRUE if the PIT is to be enabled, FALSE if the PIT is to be disabled.
 */
void PIT_Enable(const bool enable){
  if (enable)
  {
    PIT_TCTRL0 = PIT_TCTRL_TEN_MASK;       // enable timer0 interrupt
  }
  else
  {
    PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;      // disable Timer 0 interrupts
  }
}

/*! @brief Interrupt service routine for the PIT.
 *
 *  The periodic interrupt timer has timed out.
 *  The user callback function will be called.
 *  @note Assumes the PIT has been initialized.
 */
void __attribute__ ((interrupt)) PIT_ISR(void)
{
  OS_ISREnter();

  // Clear the flag
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;

  Analog_Get(VOLT_CHANNEL ,&Meter_Voltage);
  Analog_Get(CURR_CHANNEL, &Meter_Current);

  if (DAC_TestMode)
    DAC_Callback();
  if (PITCallback)
    (*PITCallback)(PITArguments);

  OS_ISRExit();
}

