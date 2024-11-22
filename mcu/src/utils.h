#pragma once

#include <stdint.h>
#include "data.h"

namespace utils {

// Performs software reset
void DoSoftReset();

// Performs initial MCU initialization: ports, ADC, timers, etc.
void InitMcu();

// Delays for the specified number of 10 ms ticks
void Delay(uint8_t n10msTicks);

// Converts x1000 voltage to the XX.XXXV string (6 chars)
void VoltageToString(uint16_t x1000Voltage, bool bLeadingSpace);

// Converts x1000 current to the X.XXA string (5 chars)
void CurrentToString(uint16_t x1000Current);

// Converts x100 wattage to the XXX.X string (5 chars)
void WattageToString(uint16_t x1000Voltage, uint16_t x1000Current);

// Converts internal integrated current value to XX.XXXAh or XXX.XXAh string (8 chars)
void CapacityToString();

// Converts elapsed time to HH-MM-SS string (8 chars)
void TimeToString();

// Resets time and capacity
void TimeCapacityReset();

// Converts x100 temperature to the XXX.X string 
void TemperatureToString(int16_t x100Temp);

// Returns color that is used to draw board and battery temperature values
uint16_t GetBoardTempColor(uint16_t temperature);
uint16_t GetBatteryTempColor(uint16_t temperature);

// Converts percent value to the XXX% string (4 chars)
void PercentToString(uint8_t percent);

// Converts unsigned 8-bit integer to string with leading spaces
void I8ToStringSpaces(uint8_t value);

// Converts signed 8-bit value to string
void I8SToString(int8_t value);

uint16_t ChangeI16ByDigit(uint16_t value, uint8_t digit, int8_t delta, uint16_t minValue, uint16_t maxValue);
uint8_t ChangeI8ByDelta(uint8_t value, int8_t delta, int8_t minValue, int8_t maxValue);

void ClearPendingKeys();

// Assembler routines
extern "C" {

// Shifts supplied operand right by 12 bits (unfortunately GCC implements this as 12x4 shift
// commands in a loop, so we have to implement it in assembler)
uint16_t ShiftRight12(uint32_t value);
uint32_t ShiftLeft12(uint16_t value);

// Converts 16-bit integer to its decimal representation. Can substitute leading zeros with spaces
void I16ToString(uint16_t value, char *buffer, uint8_t maxLeadingSpaces);

// Converts unsigned 8-bit integer to string
void I8ToString(uint8_t value, char *buffer);

// Gets 32-bit current sum divided by 4M (see timer_int.S for details)
uint32_t GetCurrentSumDiv4M();

// Returns the current encoder delta/key and resets them
int8_t GetEncoderDelta();
EEncoderKey GetEncoderKey();

// Converts TMP100 temperature to x100 value
int16_t TemperatureToDisplayX100(int16_t temperture);

} // extern "C"

} // namespace utils
