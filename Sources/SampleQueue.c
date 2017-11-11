#include "SampleQueue.h"

/*! @brief Initialize the SampleQueue before first use
 *
 *  @param queue pointer to the queue to be initialized
 */
void SQ_Init(SampleQueue* queue)
{
  uint8_t i;
  queue->oldest = 0;
  queue->newest = 0;
  queue->sum = 0;
  queue->nb = 0;
  queue->firstTime = true;
  for (i = 0; i < ARRAY_SIZE; i ++)
    queue->data[i] = 0;
}

/*! @brief Puts a number into the queue and does some necessary things
 *
 *  @param queue A pointer to a SampleQueue where data is to be stored
 *  @param data A 32-bit unsigned integer to be stored in the SampleQueue
 */
void SQ_Put(SampleQueue* queue, uint32_t sample)
{
  if (queue->nb == ARRAY_SIZE)
  // Array is full
  {
    // Replace the oldest value with new value in both data array and sum
    queue->sum -= queue->data[queue->oldest];
    queue->sum += sample;
    // Advance the oldest value pointer
    queue->newest = queue->oldest;
    queue->data[queue->oldest ++] = sample;

    // Go back to 0 if reaches end
    if (queue->oldest == ARRAY_SIZE)
      queue->oldest = 0;
    if (queue->newest == ARRAY_SIZE)
      queue->newest = 0;
  }
  else
  // Array is not full, add new values
  {
    queue->sum += sample;
    queue->data[queue->newest ++] = sample;
    if (queue->newest == ARRAY_SIZE)
      queue->newest = 0;
    queue->nb ++;
  }
}
