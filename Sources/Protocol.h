#ifndef PROTOCOL_H
#define PROTOCOL_H

/*! @brief Initialize protocol module before first use
 *
 */
void Protocol_Init();

/*! @brief Get what a second represents
 *
 *  @param ratio multiply seconds with the ratio to reduce precision lose
 *  @return result
 */
uint32_t Protocol_GetTime(uint8_t ratio);
#endif
