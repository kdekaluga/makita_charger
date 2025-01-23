#pragma once

#include "../charger_profile.h"

namespace screen::charger {

enum class EState : uint8_t
{
    NO_BATTERY = 0,
    INVALID_BATTERY,
    INVALID_BATTERY2,
    MEASURING_VOLTAGE,
    CHARGING,
    CHARGE_COMPLETE,
    BATTERY_ERROR,

    // Do nothing
    DO_NOTHING = 0x0E,

    // Don't change the current state, just reset the tick counter
    RESET_TICKS = 0x0F,

    STATE_MASK = 0x0F,

    // Don't erase background to prevent flickering
    DONT_ERASE_BACKGROUND = 0x80,
};

inline EState operator ~(EState op1)
{
    return static_cast<EState>(~static_cast<uint8_t>(op1));
}

inline EState operator &(EState op1, EState op2)
{
    return static_cast<EState>(static_cast<uint8_t>(op1) & static_cast<uint8_t>(op2));
}

inline EState operator |(EState op1, EState op2)
{
    return static_cast<EState>(static_cast<uint8_t>(op1) | static_cast<uint8_t>(op2));
}

var EState g_state;
var uint16_t g_ticksInState;

// Makita battery message
var uint8_t g_batteryMessage[32];

// Used internally by StateMachine()
var uint16_t g_previousBatteryVoltage;
var uint8_t g_noBatteryDetectCount;
var bool g_chargeCanBeFinished;

// Set in StateMachine(), used by DrawBattery()
var uint8_t g_batteryChargePercent;
var uint8_t g_batteryChargePixels;

// Updated in DrawElements()
var int16_t g_batteryChargeBarPosition;

// Set in SetWorkingOutputValues()
var uint16_t g_chargeFinishCurrentThreshold;

// Set in SetNoBatteryOuputValues()
var uint16_t g_openCurrentCorrected;
var uint16_t g_noBatteryThresholdCurrent;

// Used by SmoothValue()
var int8_t g_smoothVoltageTrend;
var uint16_t g_smoothVoltageValue;
var int8_t g_smoothCurrentTrend;
var uint16_t g_smoothCurrentValue;

void Show();

} // namespace screen::charger

