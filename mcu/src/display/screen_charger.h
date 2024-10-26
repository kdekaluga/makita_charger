#pragma once

#include "../charger_profile.h"

namespace screen::charger {

var ::charger::EState g_state;
var uint16_t g_ticksInState;

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

