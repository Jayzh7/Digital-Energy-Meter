#ifndef DAC_H
#define DAC_H

#include "types.h"

extern bool DAC_TestMode;

/*! @brief Set up DAC before first use
 *
 */
bool DAC_Init();

/*! @brief Start DAC, basically set boolean to true
 *
 */
void DAC_Start();

/*! @brief Stop DAC, basically set boolean to false
 *
 */
void DAC_Stop();

/*! @brief Set Voltage Amplitude for DAC
 *
 *  @param steps steps away from minimum voltage amplitude
 */
void DAC_SetVoltageAmp(uint16_t steps);

/*! @brief Set Phase for DAC
 *
 *  @param steps away from minimum phase
 */
void DAC_SetPhase(uint8_t steps);

/*! @brief Set Current Amplitude for DAC
 *
 *  @param steps steps away from minimum Current amplitude
 */
void DAC_SetCurrentAmp(uint16_t steps);

/*! @brief Get mode of DAC
 *
 *  @return uint8_t 1 running
 */
uint8_t DAC_GetMode();

/*! @brief Call back function of DAC, basically signal the semaphore to generate wave
 *
 */
void DAC_Callback();

#endif
