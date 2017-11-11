#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include "types.h"
#include "FTM.h"

/*! @brief Initialize debounce before first use
 *
 *  @return bool if initialized succussfully, true
 */
bool Debounce_Init(TFTMChannel channel, void(*userFunction)(void*), void* userArguments);

/*! @brief Interrupt service routine for Push button.
 *
 */
void __attribute__ ((interrupt)) Debounce_SW1_ISR(void);

#endif
