#ifndef TARIFF_H
#define TARIFF_H

#include "types.h"

typedef enum{
  TARIFF_NULL,
  TARIFF_1,
  TARIFF_2,
  TARIFF_3
}TTariff;

/*! @brief Initialize the Tariff module before first use
 *
 *  @return bool - true if Tariff is initialized successfully
 */
bool Tariff_Init(void);

/*! @brief Set the Tariff
 *
 *  @param nb mode to be set
 *  @return bool - true if tariff is set successfully
 */
bool Tariff_Set(uint8_t nb);

/*! @brief Get mode of Tariff
 *
 *  @return uint_8 mode of Tariff
 */
uint8_t Tariff_GetMode(void);

/*! @brief Get rate for now
 *
 *  @return uint32_t Rate of current time in 32Q16 format
 */
uint32_t Tariff_GetRate(void);


#endif


