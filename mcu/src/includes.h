// Common cpp include header

#pragma once

#define LOBYTE(n) ((n) & 0xFF)
#define HIBYTE(n) ((n) >> 8)
#define ABS(n) ((n) < 0 ? -(n) : (n))

#define MAX_VOLTAGE 24000
#define MAX_CURRENT 6000

#include <avr/pgmspace.h>
#include <stdint.h>

#include "common.h"
#include "data.h"
#include "utils.h"
#include "display/display.h"
#include "display/screen_power_supply.h"
#include "display/screen_charger.h"
#include "twi/twi.h"
#include "charger_profile.h"

void CheckForFailures();
void ProcessEncoderButton();
void ProcessKeyBeep();
void RequestTemperature();
extern "C" void Timer100Hz();
