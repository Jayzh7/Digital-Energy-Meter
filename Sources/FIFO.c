/*! @file
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  This contains the structure and "methods" for accessing a byte-wide FIFO.
 *
 *  @author Zhengjie Huang
 *  @date 2017-07-30
 */
#include "FIFO.h"

/*! @brief Initialize the FIFO before first use.
 *
 *  @param FIFO A pointer to the FIFO that needs initializing.
 *  @return void
 */
void FIFO_Init(TFIFO* const FIFO){
  //
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
  FIFO->BufferAccess = OS_SemaphoreCreate(1);
  FIFO->SpaceAvailbale = OS_SemaphoreCreate(FIFO_SIZE);
  FIFO->ItemsAvailable = OS_SemaphoreCreate(0);
  for(int i = 0; i < FIFO_SIZE; i++)
  {
    FIFO->Buffer[i] = 0;
  }
}

/*! @brief Put one character into the FIFO.
 *
 *  @param FIFO A pointer to a FIFO struct where data is to be stored.
 *  @param data A byte of data to store in the FIFO buffer.
 *  @return bool - TRUE if data is successfully stored in the FIFO.
 *  @note Assumes that FIFO_Init has been called.
 */
bool FIFO_Put(TFIFO* const FIFO, const uint8_t data){

  OS_SemaphoreWait(FIFO->SpaceAvailbale, 0);
  OS_SemaphoreWait(FIFO->BufferAccess, 0);

  FIFO->Buffer[FIFO->End] = data;
  FIFO->End += 1;

  //move the cursor to start
  if (FIFO->End == FIFO_SIZE)
  {
    FIFO->End = 0;
  }
  FIFO->NbBytes += 1;

  OS_SemaphoreSignal(FIFO->BufferAccess);
  OS_SemaphoreSignal(FIFO->ItemsAvailable);

  return true;
}

/*! @brief Get one character from the FIFO.
 *
 *  @param FIFO A pointer to a FIFO struct with data to be retrieved.
 *  @param dataPtr A pointer to a memory location to place the retrieved byte.
 *  @return bool - TRUE if data is successfully retrieved from the FIFO.
 *  @note Assumes that FIFO_Init has been called.
 */
bool FIFO_Get(TFIFO* const FIFO, uint8_t* const dataPtr){

  OS_SemaphoreWait(FIFO->ItemsAvailable, 0);
  OS_SemaphoreWait(FIFO->BufferAccess, 0);

  *dataPtr = FIFO->Buffer[FIFO->Start];

  FIFO->Start += 1;

  //move the cursor to start
  if (FIFO->Start == FIFO_SIZE)
  {
    FIFO->Start = 0;
  }

  FIFO->NbBytes -= 1;

  OS_SemaphoreSignal(FIFO->BufferAccess);
  OS_SemaphoreSignal(FIFO->SpaceAvailbale);

  return true;
}



