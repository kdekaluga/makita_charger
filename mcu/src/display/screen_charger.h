#pragma once

#include "../charger_profile.h"

namespace screen::charger {

using ::charger::EState;

var ::charger::SProfile g_profile;

var EState g_state;
var uint16_t g_ticksInState;

var uint8_t g_batteryChargePercent;
var uint8_t g_batteryChargePixels;
var int16_t g_batteryChargeBarPosition;

var uint16_t g_noBatteryVoltage;
var uint16_t g_openCurrentCorrected;
var uint16_t g_chargeFinishCurrentThreshold;

var uint8_t g_noBatteryDetectCount;
var bool g_chargeCanBeFinished;

var int8_t g_smoothVoltageTrend;
var uint16_t g_smoothVoltageValue;
var int8_t g_smoothCurrentTrend;
var uint16_t g_smoothCurrentValue;

void Show();




} // namespace screen::charger

