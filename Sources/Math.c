#include "Math.h"

#define NB_ITERATIONS 25

/*! @brief calculate the absolute value of a 16-bit signed integer
 *
 *  @param number signed integer
 *  @return uint16_t absolute value
 */
uint16_t Math_Abs(int16_t number)
{
  if (number & 0x8000)
  // The number is negative
  {
    // Invert the bits
    number ^= 0xffff;
    // Add one
    number ++;
  }

  return (uint16_t)number;
}

/*! @biref Calculate the square root of a number
 *
 *  @param estimate estimate value of the square root value. (0 stands for no value provided)
 *  @param number number to be calculated
 *  @param iterateTimes times to iterate using Newton's method
 *  @return uint32_t square root value
 */
uint32_t Math_SquareRoot(uint16_t estimate, uint32_t number, uint8_t iterateTimes)
{
  uint8_t i;
  uint32_t newEstimate;

  // Check if estimate is provided.
  if (estimate == 0)
  {
    // No estimate
    newEstimate = number;
    // Right shift until estimate doesn't exceed 16 bits
    while (newEstimate > 0xffff)
    {
      newEstimate = newEstimate >> 1;
    }
    estimate = (uint16_t)newEstimate;
  }

  // No iteration times provided. Use default value
  if (iterateTimes == 0)
    iterateTimes = NB_ITERATIONS;

  // Estimate square root using Newton's method
  for (i = 0; i < iterateTimes; i ++)
  {
    if (estimate == 0)
      return 0;
    estimate = (number/estimate + estimate) / 2;
  }

  return estimate;
}


