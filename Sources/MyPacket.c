/*! @file
 *
 *  @brief Routines to implement packet encoding and decoding for the serial port.
 *
 *  This contains the functions for implementing the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author Zhengjie Huang
 *  @date 2017-07-30
 */

#include "MyPacket.h"
#include "UART.h"
#include "Flash.h"
#include "Cpu.h"
#include "OS.h"

OS_ECB* PacketSemaphore;

uint8_t NbBytesInPkt;

/*! @brief Initializes the packets by calling the initialization routines of the supporting software modules.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz
 *  @return bool - TRUE if the packet module was successfully initialized.
 */
bool MyPacket_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  NbBytesInPkt = 0;
  PacketSemaphore = OS_SemaphoreCreate(1);
  return UART_Init(baudRate, moduleClk);
}

/*! @brief Attempts to get a packet from the received data.
 *
 *  @return bool - TRUE if a valid packet was received.
 */
bool MyPacket_Get(void)
{
  uint8_t  dataPtr;
  if (UART_InChar(&dataPtr) == true)
  {
//    EnterCritical();
    switch(NbBytesInPkt)
    {
      case 0:
        Packet_Command = dataPtr;
        NbBytesInPkt ++;
        break;
      case 1:
        Packet_Parameter1 = dataPtr;
        NbBytesInPkt ++;
        break;
      case 2:
        Packet_Parameter2 = dataPtr;
        NbBytesInPkt ++;
        break;
      case 3:
        Packet_Parameter3 = dataPtr;
        NbBytesInPkt ++;
        break;
      case 4:
        Packet_Checksum = dataPtr;
        if (Packet_Checksum == (Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3))
        {
          //checksum OK
          NbBytesInPkt = 0;
          //Packet_Command = dataPtr;
          return true;
        }
        else
        {
          //checksum bad, discard one byte and shift.
          Packet_Command = Packet_Parameter1;
          Packet_Parameter1 = Packet_Parameter2;
          Packet_Parameter2 = Packet_Parameter3;
          Packet_Parameter3 = Packet_Checksum;
          NbBytesInPkt = 4;
        }
      default: // Clear all bytes
        NbBytesInPkt = 0;
    }
//    ExitCritical();
  }
  return false;
}



/*! @brief Builds a packet and places it in the transmit FIFO buffer.
 *
 *  @return bool - TRUE if a valid packet was sent.
 */
bool MyPacket_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3){
  OS_SemaphoreWait(PacketSemaphore, 0);

  // Send command and parameters
  if (UART_OutChar(command) == false)
  {
    return false;
  }
  if (UART_OutChar(parameter1) == false)
  {
    return false;
  }
  if (UART_OutChar(parameter2) == false)
  {
    return false;
  }
  if (UART_OutChar(parameter3) == false)
  {
    return false;
  }
  uint8_t checksum = command ^ parameter1 ^ parameter2 ^ parameter3;
  if (UART_OutChar(checksum) == false)
  {
    return false;
  }

  OS_SemaphoreSignal(PacketSemaphore);
  return true;
}

