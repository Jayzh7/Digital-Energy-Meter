#ifndef METER_H
#define METER_H

#include "types.h"

#define CYCLES_PER_SECOND 50
#define SAMPLES_PER_CYCLE 16

// 16 samples per cycle(50Hz) which is 800 Hz
// Tick every 1.25 ms which is 1250000 ns
#define SAMPLE_PERIOD 1250000
// 0.1 Hz for 16 samples per cycle
#define SAMPLE_TIME_INCREMENT 2506

extern int16_t Meter_Voltage;
extern int16_t Meter_Current;

extern uint16_t Meter_VoltageRMS;
extern uint16_t Meter_CurrentRMS;


extern uint32_t Meter_AveragePower;  /*!< P=VIcos, calculated every cycle(after sampling current and voltage 16 times)  */
extern uint64_t Meter_Energy;        /*!< E=sum(p)*Ts, calculated every period */
extern uint64_t Meter_Cost;          /*!< Cost of electricity, calculated every period */
extern uint16_t Meter_PowerFactor;

/*! @brief Initialize meter module by creating threads and enabling timer.
 *  @param moduleClk The module clock rate in Hz.
 */
bool Meter_Init(const uint32_t moduleClk);

/*! @brief Get the frequency difference between current frequency and 1/(16*47.5)Hz
 *
 *  @return uint8_t difference, from 0 to 50, represents 1/(16*52.5) to 1/(16*47.5) respectively.
 */
uint8_t Meter_GetFrequencyDiff();
#endif
