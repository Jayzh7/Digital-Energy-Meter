#include "Debounce.h"
#include "Cpu.h"
#include "OS.h"
#include "FTM.h"

static void (*ButtonPushedFunc)(void*);
static void (*ButtonPushedArgs);

/*! @brief Initialize debounce before first use
 *
 *  @return bool if initialized succussfully, true
 */
bool Debounce_Init(TFTMChannel channel, void(*userFunction)(void*), void* userArguments)
{
  ButtonPushedFunc = userFunction;
  ButtonPushedArgs = userArguments;

  // Enable PORT_D Clock Gate
  SIM_SCGC5  |= SIM_SCGC5_PORTD_MASK;

  PORTD_PCR0 &= ~PORT_PCR_MUX_MASK;  // Clear MUX bits
  PORTD_PCR0 |= PORT_PCR_MUX(1);     // select GPIO

  PORTD_PCR0 |= PORT_PCR_ISF_MASK;  // Clear interrupts
  PORTD_PCR0 |= PORT_PCR_IRQC(10);  // Flag and Interrupt on falling-edge.

  PORTD_PCR0 |= PORT_PCR_PE_MASK;   // Enable resistor
  PORTD_PCR0 |= PORT_PCR_PS_MASK;   // Select pullup resistor

  // Initialize NVIC
  // Clear any pending interrupts on PORTD
  NVICICPR2  |= (1 << 26);
  // Enable interrupts from PORTD
  NVICISER2  |= (1 << 26);

  return FTM_Set(&channel);
}


/*! @brief Interrupt service routine for Push button.
 *
 */
void __attribute__ ((interrupt)) Debounce_SW1_ISR(void)
{
  OS_ISREnter();

  PORTD_ISFR |= PORT_ISFR_ISF(0);       // Clear the SW1 interrupt flag
  PORTD_PCR0 &= ~PORT_PCR_IRQC_MASK;    // Disable interrupt

  if (ButtonPushedFunc)
    ButtonPushedFunc(ButtonPushedArgs);

  PORTD_PCR0 |= PORT_PCR_IRQC(10);  // Flag and Interrupt on falling-edge.
  OS_ISRExit();
}
