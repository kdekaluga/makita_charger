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

#define MAX_VOLTAGE 24000
#define MAX_CURRENT 6000

#include <avr/pgmspace.h>
#include <stdint.h>

#include "common.h"
#include "data.h"
#include "charger_profile.h"
#include "utils.h"
#include "twi/twi.h"
#include "display/display.h"
#include "display/screen_power_supply.h"
#include "display/screen_charger.h"
#include "display/screen_calibration.h"
#include "display/screen_music_player.h"
#include "display/screen_settings.h"
#include "sound/music.h"

void CheckForFailures();
void ProcessEncoderButton();
void RequestTemperature();
extern "C" void Timer100Hz();
