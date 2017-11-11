#ifndef SAMPLE_QUEUE_H
#define SAMPLE_QUEUE_H

#include "types.h"
#include "meter.h"

#define ARRAY_SIZE SAMPLES_PER_CYCLE

typedef struct
{
  bool firstTime;
  uint8_t oldest;
  uint8_t newest;
  uint32_t data[ARRAY_SIZE]; /*!< The samples are stored as a 32Q16 format */
  uint32_t latestValue; /*!< Latest voltage or current value in 32Q16 */
  uint64_t sum;  /*!< Sum of data array */
  uint8_t nb;    /*!< Number of data in array */
}SampleQueue;

/*! @brief Initialize the SampleQueue before first use
 *
 *  @param queue pointer to the queue to be initialized
 */
void SQ_Init(SampleQueue* queue);

/*! @brief Puts a number into the queue and does some necessary things
 *
 *  @param queue A pointer to a SampleQueue where data is to be stored
 *  @param data A 32-bit unsigned integer to be stored in the SampleQueue
 */
void SQ_Put(SampleQueue* queue, uint32_t data);

#endif
