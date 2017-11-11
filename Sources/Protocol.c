#include "MyPacket.h"
#include "OS.h"
#include "Protocol.h"
#include "Tariff.h"
#include "Cpu.h"
#include "DAC.h"
#include "meter.h"
#include "MyRTC.h"

#define THREAD_STACK_SIZE 100

// Basic Protocol
#define CMD_TEST   0x10
#define CMD_TARIFF 0x11
#define CMD_TIME1  0x12
#define CMD_TIME2  0x13
#define CMD_POWER  0x14
#define CMD_ENERGY 0x15
#define CMD_COST   0x16

// Intermediate Protocol
#define CMD_FREQUENCY    0x17
#define CMD_VOLTAGE_RMS  0x18
#define CMD_CURRENT_RMS  0x19
#define CMD_POWER_FACTOR 0x1A

// Other protocol
#define CMD_VOLTAGE_AMP  0x1B
#define CMD_CURRENT_AMP  0x1C
#define CMD_PHASE        0x1D

OS_THREAD_STACK(ProtocolThreadStack, THREAD_STACK_SIZE); /*!< The stack for the Packet Handle thread. */

static bool TestMode = false;

uint32_t Meter_AveragePower;
uint64_t Meter_Energy;
uint64_t Meter_Cost;
uint16_t Meter_PowerFactor;

uint16_t Meter_VoltageRMS;
uint16_t Meter_CurrentRMS;


bool HandleTariff()
{
  // Get Tariff
  if (Packet_Parameter2 == 1)
    return MyPacket_Put(CMD_TARIFF, Tariff_GetMode(), Packet_Parameter2, Packet_Parameter3);

  // Tariff range out of scope
  if (!(Packet_Parameter1 == 3 || Packet_Parameter1 == 1 || Packet_Parameter1 == 2))
    return false;

  if (Packet_Parameter2 == 0)
    return Tariff_Set((uint8_t)Packet_Parameter1);

  return false;
}

bool HandleTest()
{
  if (Packet_Parameter2 == 0)
  {
    TestMode = (bool)Packet_Parameter1;
    if (TestMode)
      DAC_Start();
    else
      DAC_Stop();
    return true;
  }

  if (Packet_Parameter2 == 1)
  {
    return MyPacket_Put(CMD_TEST, DAC_GetMode(), Packet_Parameter2, Packet_Parameter3);
  }

  return false;
}

bool HandleTime1()
{
  if (Packet_Parameter1 > 59 || Packet_Parameter2 > 59)
    return false;

  MyRTC_Set1(Packet_Parameter2, Packet_Parameter1);
  return true;
}

bool HandleTime2()
{
  if (Packet_Parameter1 > 23)
    return false;

  MyRTC_Set2(Packet_Parameter2, Packet_Parameter1);
  return true;
}

bool HandlePower()
{
  uint16union_t power;
  power.l = (uint16_t)((Meter_AveragePower*1000) >> 16);
  return MyPacket_Put(CMD_POWER, power.s.Lo, power.s.Hi, 0);
}

bool HandleEnergy()
{
  uint16union_t energy;
  energy.l = (uint16_t)(((uint32_t)(Meter_Energy>>16) / 3600) >> 16);
  return MyPacket_Put(CMD_ENERGY, energy.s.Lo, energy.s.Hi, 0);
}

bool HandleCost()
{
  uint32_t totalCents = (uint32_t)(Meter_Cost >> 32);
  uint8_t cents = totalCents % 100;
  uint16union_t dollars;
  dollars.l = cents/100;

  return MyPacket_Put(CMD_COST, cents, dollars.s.Lo, dollars.s.Hi);
}

bool HandleFrequency()
{
  uint16union_t freq;
  freq.l = 525 - Meter_GetFrequencyDiff();
  return MyPacket_Put(Packet_Command, freq.s.Lo, freq.s.Hi, 0);
}

bool HandleVoltageRMS()
{
  uint16union_t volt;
  volt.l = Meter_VoltageRMS;

  return MyPacket_Put(Packet_Command, volt.s.Lo, volt.s.Hi, 0);
}

bool HandleCurrentRMS()
{
  uint16union_t curr;
  curr.l = Meter_CurrentRMS;

  return MyPacket_Put(Packet_Command, curr.s.Lo, curr.s.Hi, 0);
}

bool HandlePowerFactor()
{
  uint16union_t pf;
  pf.l = (uint16_t)(((uint32_t)Meter_PowerFactor*1000) >> 8);

  return MyPacket_Put(Packet_Command, pf.s.Lo, pf.s.Hi, 0);
}

bool HandleVoltageAmp()
{
  uint16union_t steps;
  steps.s.Lo = Packet_Parameter1;
  steps.s.Hi = Packet_Parameter2;

  if (steps.l > 2317)
    return false;

  DAC_SetVoltageAmp(steps.l);
  return true;
}

bool HandleCurrentAmp()
{
  uint16union_t steps;
  steps.s.Lo = Packet_Parameter1;
  steps.s.Hi = Packet_Parameter2;

  if (steps.l > 23170)
    return false;

  DAC_SetCurrentAmp(steps.l);
  return true;
}

bool HandlePhase()
{
  if (Packet_Parameter1 > 32)
    return false;

  DAC_SetPhase(Packet_Parameter1);
  return true;
}

static void HandlePacket()
{
  static bool success;

  if (MyPacket_Get())
  {
    switch (Packet_Command)
    {
      case CMD_TARIFF:
        success = HandleTariff();
        break;
      case CMD_TEST:
        success = HandleTest();
        break;
      case CMD_TIME1:
        success = HandleTime1();
        break;
      case CMD_TIME2:
        success = HandleTime2();
        break;
      case CMD_POWER:
        success = HandlePower();
        break;
      case CMD_ENERGY:
        success = HandleEnergy();
        break;
      case CMD_COST:
        success = HandleCost();
        break;

      // Intermediate protocol
      case CMD_FREQUENCY:
        success = HandleFrequency();
        break;
      case CMD_VOLTAGE_RMS:
        success = HandleVoltageRMS();
        break;
      case CMD_CURRENT_RMS:
        success = HandleCurrentRMS();
        break;
      case CMD_POWER_FACTOR:
        success = HandlePowerFactor();
        break;

      case CMD_VOLTAGE_AMP:
        success = HandleVoltageAmp();
        break;
      case CMD_CURRENT_AMP:
        success = HandleCurrentAmp();
        break;
      case CMD_PHASE:
        success = HandlePhase();
        break;
    }
  }
}

/*! @brief Thread to handle packets received
 *
 *  @param pData Thread data
 */
void ProtocolThread(void* pData)
{
  for (;;)
  {
    HandlePacket();
  }
}

/*! @brief RTC callback used to manipulate time increment
 *
 */
void RTCCallback()
{
  if (TestMode)
  {
    // disable time counter
    RTC_SR &= ~RTC_SR_TCE_MASK;

    //set time in TSR
    RTC_TSR += 3599;

    //enable time counter
    RTC_SR |= RTC_SR_TCE_MASK;
  }
}

/*! @brief Initialize protocol module before first use
 *
 */
void Protocol_Init()
{
  MyRTC_Init(RTCCallback, NULL);
  OS_ThreadCreate(ProtocolThread,
                  NULL,
                  &ProtocolThreadStack[THREAD_STACK_SIZE - 1],
                  5);
}

/*! @brief Get what a second represents
 *
 *  @param ratio multiply seconds with the ratio to reduce precision lose
 *  @return result
 */
uint32_t Protocol_GetTime(uint8_t ratio)
{
  // Returns a 32Q16 number
  if (TestMode)
  {
    // Accelerated time: 0.00125s*3600
    return (uint32_t)((0.00125 * 65536 * 3600) * ratio);
  }
  else
  {
    // Normal time: 0.00125s
    return (uint32_t)((0.00125 * 65536) * ratio);
  }
}
