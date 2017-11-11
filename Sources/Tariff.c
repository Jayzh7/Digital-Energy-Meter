#include "Tariff.h"
#include "Flash.h"
#include "MyRTC.h"

// 22.235 cents/kWh, left shift 16 bits = 1457192.96
static uint32_t const PEAK_RATE     = 1457193;
static uint32_t const SHOULDER_RATE = 288358;
static uint32_t const OFFPEAK_RATE  = 138215;

static uint32_t const TARIFF2_RATE  = 112263;
static uint32_t const TARIFF3_RATE  = 268698;

static TTariff CurrentTariff;

bool isSet(uint8_t data)
{
  return (data == 3 || data == 1 || data == 2);
}

/*! @brief Initialize the Tariff module before first use
 *
 *  @return bool - true if Tariff is initialized successfully
 */
bool Tariff_Init()
{
  if (Flash_Init())
  {
    uint8_t data = (uint8_t)(_FB(FLASH_DATA_START));
    if (!isSet(data))
    {
      CurrentTariff = TARIFF_1;
      return Flash_Write8((uint8_t*)FLASH_DATA_START, 1);
    }
    else
    {
      CurrentTariff = (TTariff)data;
      return true;
    }
  }
  return false;
}

/*! @brief Set the Tariff
 *
 *  @param nb mode to be set
 *  @return bool - true if tariff is set successfully
 */
bool Tariff_Set(uint8_t nb)
{
  if ((uint8_t)CurrentTariff == nb)
     return true;

  if (Flash_Write8((uint8_t*)FLASH_DATA_START, nb))
  {
    CurrentTariff = (TTariff)nb;
    return true;
  }

  return false;
}

/*! @brief Get mode of Tariff
 *
 *  @return uint_8 mode of Tariff
 */
uint8_t Tariff_GetMode()
{
  return (uint8_t)CurrentTariff;
}

/*! @brief Get rate for now
 *
 *  @return uint32_t Rate of current time in 32Q16 format
 */
uint32_t Tariff_GetRate()
{
  uint8_t day, hour, minute, second;
  switch (CurrentTariff)
  {
    case TARIFF_1:
      MyRTC_Get(&day, &hour, &minute, &second);
      if (hour < 7 || hour >= 22)
        return OFFPEAK_RATE;
      else if (hour < 14 || hour >= 20)
        return SHOULDER_RATE;
      else
        return PEAK_RATE;
    case TARIFF_2:
      return TARIFF2_RATE;
    case TARIFF_3:
      return TARIFF3_RATE;
    default:
      // Something went wrong
      return 0;
  }
}
