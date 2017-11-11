#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"

/*! @brief Initialize Display module before first use
 *         Basically create a semphore for display thread
 */
void Display_Init();

/*! @brief Display metering time
 *
 */
void Display_MeteringTime();

/*! @brief Display average power
 *
 */
void Display_AveragePower();

/*! @brief Display total energy
 *
 */
void Display_TotalEnergy();

/*! @brief Display total cost
 *
 */
void Display_TotalCost();

#endif


