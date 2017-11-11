/*! @file
 *
 *  @brief Routines for controlling the Real Time Clock (RTC) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the real time clock (RTC).
 *
 *  @author Zhengjie Huang
 *  @date 2017-09-05
 */

// new types
#include "MyRTC.h"
#include "LEDs.h"
#include "Cpu.h"
#include "OS.h"

static void (*RTCCallback)(void*);
static void *RTCArugments;

/*! @brief Initializes the RTC before first use.
 *
 *  Sets up the control register for the RTC and locks it.
 *  Enables the RTC and sets an interrupt every second.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the RTC was successfully initialized.
 */
bool MyRTC_Init(void (*userFunction)(void*), void* userArguments)
{
  RTCCallback = userFunction;
  RTCArugments = userArguments;

  //enable the clock to RTC module register space
  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

  //software reset
  RTC_CR |= RTC_CR_SWR_MASK;
  RTC_CR  &= ~RTC_CR_SWR_MASK;

  //oscillator enable
  RTC_CR |= (RTC_CR_OSCE_MASK | RTC_CR_SC2P_MASK | RTC_CR_SC16P_MASK);

  //Wait until the oscillator output is stable
//  for(int i = 0; i < 0x5000; i ++) ;

  // Initialize NVIC
  // Vector 83, IRQ=67
  // NVIC non-IPR=2 IPR=16
  // Clear any pending interrupts on RTC
  NVICICPR2 = (1 << 3);
  // Enable interrupts from RTC module
  NVICISER2 = (1 << 3);

  //enable interrupt
  RTC_IER = RTC_IER_TSIE_MASK;
  //enable time counter
  RTC_SR |= RTC_SR_TCE_MASK;
}

/*! @brief Sets the value of the real time clock.
 *
 *  @param minutes The desired value of the real time clock minutes (0-59).
 *  @param seconds The desired value of the real time clock seconds (0-59).
 *  @note Assumes that the RTC module has been initialized and all input parameters are in range.
 */
void MyRTC_Set1(const uint8_t minutes, const uint8_t seconds)
{
  uint32_t oldTimeInSeconds = MyRTC_GetTimeInSeconds();

  uint8_t oldTimeInHours = (oldTimeInSeconds/3600);

  uint32_t newTimeInSeconds = oldTimeInHours*3600 + minutes*60 + seconds;

  RTC_SR &= ~RTC_SR_TCE_MASK;  // disable time counter
  RTC_TSR = newTimeInSeconds; // set time in TSR
  RTC_SR |= RTC_SR_TCE_MASK;   // enable time counter
}

/*! @brief Sets the value of the real time clock.
 *
 *  @param days The desired value of the real time clock days (0-255).
 *  @param hours The desired value of the real time clock hours (0-23).
 *  @note Assumes that the RTC module has been initialized and all input parameters are in range.
 */
void MyRTC_Set2(const uint8_t days, const uint8_t hours)
{
  uint32_t oldTimeInSeconds = MyRTC_GetTimeInSeconds();

//  uint8_t oldMinutes = (oldTimeInSeconds%(3600*24))/60;
//  uint8_t oldSeconds = (oldTimeInSeconds%(3600*24))%60;

  uint32_t oldMinuteSecondInSeconds = oldTimeInSeconds%(3600);

  uint32_t newTimeInSeconds = days*86400 + hours*3600 + oldMinuteSecondInSeconds;

  RTC_SR &= ~RTC_SR_TCE_MASK;  // disable time counter
  RTC_TSR = newTimeInSeconds; // set time in TSR
  RTC_SR |= RTC_SR_TCE_MASK;   // enable time counter
}


/*! @brief Gets the value of the real time clock.
 *
 *  @param days The address of a variable to store the real time clock days.
 *  @param hours The address of a variable to store the real time clock hours.
 *  @param minutes The address of a variable to store the real time clock minutes.
 *  @param seconds The address of a variable to store the real time clock seconds.
 *  @note Assumes that the RTC module has been initialized.
 */
void MyRTC_Get(uint8_t* const days, uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
  uint32_t data = MyRTC_GetTimeInSeconds();
  *days = data/(3600*24);
  uint32_t seconds1 = (data)%(3600*24);
  *hours = seconds1/3600;
  uint32_t seconds2 = (seconds1)%3600;
  *minutes = (seconds2)/60;
  *seconds = (seconds2)%60;
}

/*! @brief Get time in seconds
 *
 *  @return uint32_t seconds
 */
uint32_t MyRTC_GetTimeInSeconds()
{
  bool timeMatch = false;
  uint32_t data1, data2;

  while (!timeMatch)
  {
    data1 = RTC_TSR;
    data2 = RTC_TSR;
    timeMatch = (data1 == data2);
  }

  return data1;
}
/*! @brief Interrupt service routine for the RTC.
 *
 *  The RTC has incremented one second.
 *  The user callback function will be called.
 *  @note Assumes the RTC has been initialized.
 */
void __attribute__ ((interrupt)) MyRTC_ISR(void)
{
  OS_ISREnter();

  if (RTCCallback)
    RTCCallback(RTCArugments);

  OS_ISRExit();
}
