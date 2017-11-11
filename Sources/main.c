/* ###################################################################
 **     Filename    : main.c
 **     Project     : Project
 **     Processor   : MK70FN1M0VMJ12
 **     Version     : Driver 01.01
 **     Compiler    : GNU C Compiler
 **     Date/Time   : 2015-07-20, 13:27, # CodeGen: 0
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @version 6.0
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup main_module main module documentation
 **  @{
 */
/* MODULE main */

// CPU module - contains low level hardware initialization routines
#include "Cpu.h"
#include "MyPacket.h"
#include "Interface.h"
#include "FTM.h"
#include "LEDs.h"
#include "Protocol.h"
#include "Tariff.h"
#include "MyRTC.h"
// Simple OS
#include "OS.h"
#include "meter.h"
#include "DAC.h"

// Analog functions
#include "analog.h"

#define MODULE_CLK CPU_BUS_CLK_HZ
#define BAUD_RATE 115200

// ----------------------------------------
// Thread set up
// ----------------------------------------
// Arbitrary thread stack size - big enough for stacking of interrupts and OS use.
#define THREAD_STACK_SIZE 200

// Thread stacks
OS_THREAD_STACK(InitModulesThreadStack, THREAD_STACK_SIZE); /*!< The stack for the LED Init thread. */

/*! @brief Initializes modules.
 *
 *  Several threads will be created.
 *  main.c:      InitModulesThread 0
 *  meter.c:     VoltageThread     3
 *               CurrentThread     4
 *               CalcThread        8
 *  UART.c:      TxThread          7
 *               RxThread          1
 *  DAC.c:       OutputThread      2
 *  Protocol.c:  ProtocolThread    5
 *  Interface.c: PushButtonThread  6
 *               DisplayThread     9
 *
 */
static void InitModulesThread(void* pData)
{
  OS_DisableInterrupts();

  // Initialize analog module
  (void)Analog_Init(CPU_BUS_CLK_HZ);

  MyPacket_Init(BAUD_RATE, MODULE_CLK);

  Tariff_Init();
  Meter_Init(MODULE_CLK);
  FTM_Init();
  DAC_Init();
  Interface_Init();
  LEDs_Init();
  Protocol_Init();
  OS_EnableInterrupts();
  // We only do this once - therefore delete this thread
  OS_ThreadDelete(OS_PRIORITY_SELF);
}


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  OS_ERROR error;

  // Initialise low-level clocks etc using Processor Expert code
  PE_low_level_init();

  // Initialize the RTOS
  OS_Init(CPU_CORE_CLK_HZ, true);
  // Create module initialisation thread
  error = OS_ThreadCreate(InitModulesThread,
                          NULL,
                          &InitModulesThreadStack[THREAD_STACK_SIZE - 1],
                          0); // Highest priority

  // Start multithreading - never returns!
  OS_Start();
}

/*!
 ** @}
 */
