// Common cpp include header

#pragma once

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define STRINGIZE1(x) #x
#define STRINGIZE(x) STRINGIZE1(x)

#define VERSION_STRING "v" STRINGIZE(VERSION_MAJOR) "." STRINGIZE(VERSION_MINOR)

#define LOBYTE(n) ((n) & 0xFF)
#define HIBYTE(n) ((n) >> 8)
#define ABS(n) ((n) < 0 ? -(n) : (n))

#define BREAK asm volatile ("BREAK")

#define MAX_VOLTAGE 24000
#define MAX_CURRENT 8000

#define TWI_ADDR_TMP100BOARD 0b01001000
#define TWI_ADDR_TMP100BATTERY 0b01001010

#define TEMP_BOARD_SYMBOL 'C'
#define TEMP_BATTERY_SYMBOL 'B'

#define DEFAULT_VOLTAGE_OFFSET static_cast<int8_t>(253)
#define DEFAULT_VOLTAGE_COEFF 24580
#define DEFAULT_CURRENT_OFFSET -2
#define DEFAULT_CURRENT_COEFF 12300

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <stdint.h>

#include "common.h"
#include "data.h"
#include "charger_profile.h"
#include "utils.h"
#include "twi/twi.h"
#include "one_wire/one_wire.h"
#include "display/display.h"
#include "display/screen_power_supply.h"
#include "display/screen_charger.h"
#include "display/screen_calibration.h"
#include "display/screen_music_player.h"
#include "display/screen_settings.h"
#include "display/screen_charger_profile.h"
#include "sound/music.h"

void CheckForFailures();
void ProcessEncoderButton();
void RequestTemperature();
void ProcessFanSpeed();
extern "C" void Timer100Hz();
