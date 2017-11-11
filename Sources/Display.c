#include "Display.h"
#include "LEDs.h"
#include "OS.h"
#include "UART.h"
#include "Tariff.h"
#include "MyRTC.h"
#include "meter.h"

#include <stdio.h>
#include <string.h>

#define JOULE_PER_KWH 3600000
OS_ECB* PrintSemaphore;

uint64_t Meter_Energy;
uint64_t Meter_Cost;
uint32_t Meter_AveragePower;

/*! @brief Print the buffer to UART
 *
 *  @param buffer The buffer to be printed
 *  @param length length of buffer
 */
void printToUART(char* const buffer, uint8_t length)
{
//  OS_SemaphoreWait(PrintSemaphore, 0);
  OS_DisableInterrupts();
  for (uint8_t i = 0; i < length; i ++)
  {
    UART_OutChar((uint8_t)buffer[i]);
  }
  OS_EnableInterrupts();
//  OS_SemaphoreSignal(PrintSemaphore);
}

/*! @brief Convert 32Q16 to xxx.xxx
 *
 *  @param data data to be converted
 *  @param pointer to the integer part
 *  @param pointer to the decimal part
 */
void convert1(uint32_t const data, uint16_t* integer, uint16_t* decimal)
{
  *integer = (uint16_t)(data >> 16);
  *decimal = ((uint16_t)data) * 1000 / 65536;
}


/*! @brief Convert 32Q16 to xxxx.xx
 *
 *  @param data data to be converted
 *  @param pointer to the integer part
 *  @param pointer to the decimal part
 */
void convert2(uint32_t const data, uint16_t* integer, uint16_t* decimal)
{
  *integer = (uint16_t)(data >> 16);
  *decimal = ((uint16_t)data) * 100 / 65536;
}

/*! @brief Initialize Display module before first use
 *         Basically create a semphore for display thread
 */
void Display_Init()
{
  PrintSemaphore = OS_SemaphoreCreate(1);
}

/*! @brief Display metering time
 *
 */
void Display_MeteringTime()
{
  char buffer[21];

  uint8_t day, hour, minute, second;
  MyRTC_Get(&day, &hour, &minute, &second);
  if (day <= 99)
  {
    snprintf(buffer, 20, "Time: %02d:%02d:%02d:%02d\n", day, hour, minute, second);
  }
  else
  {
    snprintf(buffer, 20, "Time: xx:xx:xx:xx\n");
  }

  printToUART(buffer, strlen(buffer));
}

/*! @brief Display average power
 *
 */
void Display_AveragePower()
{
  char buffer[14];

  uint16_t integer, decimal;
  convert1(Meter_AveragePower, &integer, &decimal);

  if (integer > 999)
  {
    snprintf(buffer, 14, "AP: xxx.xxx\n");
  }
  else
  {
    snprintf(buffer, 14, "AP: %d.%03d\n", integer, decimal);
  }

  printToUART(buffer, strlen(buffer));
}

/*! @brief Display total energy
 *
 */
void Display_TotalEnergy()
{
  char buffer[14];

  uint16_t integer, decimal;
  // Convert from 64Q32 to 32Q16
  convert1((uint32_t)(Meter_Energy/JOULE_PER_KWH >> 16), &integer, &decimal);

  if (integer > 999)
  {
    snprintf(buffer, 14, "TE: xxx.xxx\n");
  }
  else
  {
    snprintf(buffer, 14, "TE: %d.%03d\n", integer, decimal);
  }

  printToUART(buffer, strlen(buffer));
}

/*! @brief Display total cost
 *
 */
void Display_TotalCost()
{
  char buffer[14];

  uint16_t integer, decimal;
  convert2((uint32_t)((Meter_Cost/100) >> 16), &integer, &decimal);

  if (integer > 9999)
  {
    snprintf(buffer, 14, "TC: xxxx.xx\n");
  }
  else
  {
    snprintf(buffer, 14, "TC: %d.%02d\n", integer, decimal);
  }

  printToUART(buffer, strlen(buffer));
}




