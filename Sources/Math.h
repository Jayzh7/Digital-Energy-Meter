#ifndef MATH_H
#define MATH_H

#include "types.h"

/*! @brief calculate the absolute value of a 16-bit signed integer
 *
 *  @param number signed integer
 *  @return uint16_t absolute value
 */
uint16_t Math_Abs(int16_t number);

/*! @biref Calculate the square root of a number
 *
 *  @param estimate estimate value of the square root value. (0 stands for no value provided)
 *  @param number number to be calculated
 *  @param iterateTimes times to iterate using Newton's method
 *  @return uint32_t square root value
 */
uint32_t Math_SquareRoot(uint16_t estimate, uint32_t number, uint8_t iterateTimes);

#endif


