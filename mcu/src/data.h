#pragma once

// 'var' should be defined to the empty string in one cpp unit to instantiate variables.
#ifndef var
#define var extern
#endif

#include <stdint.h>

// General purpose buffer
var char g_buffer[16];

// Timer0 overflow interrupt counter, 0-7
var uint8_t g_timerCounter;

// Timer 625 divider counter
var uint16_t g_timer625DividerCounter;

// Output PWM value, 16 bit. The high byte is written to the timer register
// and the low byte is used to increase the PWM resolution by dithering.
var uint16_t g_pwmValue;

// The current PWM value LSB for dithering
var uint8_t g_pwmLowPos;

// ADC voltage and current accumulators
var uint16_t g_adcVoltageAcc;
var uint16_t g_adcCurrentAcc;

// The integral part of the PID algorithm
var uint8_t g_pidIntegral[3];

// Current PID mode, PID_MODE_OFF, PID_MODE_CV or PID_MODE_CC
var uint8_t g_pidMode;

// Target ADC voltage and current for PID
var uint16_t g_pidTargetVoltage;
var uint16_t g_pidTargetCurrent;

// *** ADC averager ***

// ADC averager counter
var uint8_t g_adcAveragerCounter;

// ADC voltage and current averager accumulators, 24 bit
var uint8_t g_adcAveragerVoltageAcc[3];
var uint8_t g_adcAveragerCurrentAcc[3];

// Average ADC voltage and current, 30.5 updates per second
var volatile uint16_t g_adcVoltageAverage;
var volatile uint16_t g_adcCurrentAverage;

// Total sum of every current value measured, 48 bit (for delivered capacity calculating)
var uint8_t g_totalCurrentSum[6];

// *** Encoder ***

// Last encoder state (for the main interrupt)
var uint8_t g_encoderState;

// Represents relative encoder position
var int8_t g_encoderCounter;

enum class EEncoderKey : uint8_t
{
    None = 0,
    Down,
    Up,
    DownLong,
};

var EEncoderKey g_encoderKey;

// *** 100 Hz timer ***

// 100 Hz timer counter
var volatile uint8_t g_100HzCounter;

// Time in 100Hz ticks, seconds, minutes and hours
var volatile uint8_t g_time[4];

// Controls whether output is off or on
var bool g_outOn;

// Current failure state.
// Bits 7-4 represent the current failure state and
// bits 3-0 contain internal counter
var volatile uint8_t g_failureState;

#define FAILURE_NONE 0x10
#define FAILURE_POWER_LOW 0x20
#define FAILURE_OVERVOLTAGE 0x40
#define FAILURE_OVERCURRENT 0x80
#define FAILURE_ANY (FAILURE_POWER_LOW | FAILURE_OVERVOLTAGE | FAILURE_OVERCURRENT)

// How many 100 Hz ticks of key beep remains
var uint8_t g_keyBeepLengthLeft;

// State of the temperature sensor request routine
var uint8_t g_tempRequesterState;

// Board temprerature
var uint16_t g_temperatureBoard;

// ***

struct SSettings
{
    // Key (encoder rotation and click) beep length and volume
    // (set length to 0 to disable)
    uint8_t m_keyBeepLength;
    uint8_t m_keyBeepVolume;

    // Voltage calibration values
    int8_t m_voltageOffset;
    uint16_t m_voltage4096Value;

    // Current calibration values
    int8_t m_currentOffset;
    uint16_t m_current4096Value;

    // Voltage and current settings x1000 for the power supply mode
    uint16_t m_psVoltageX1000;
    uint16_t m_psCurrentX1000;

    // Measurement conversion routines
    uint16_t AdcVoltageToDisplayX1000(uint16_t adcVoltage);
    uint16_t AdcCurrentToDisplayX1000(uint16_t adcCurrent);
    uint16_t DisplayX1000VoltageToAdc(uint16_t x1000Voltage);
    uint16_t DisplayX1000CurrentToAdc(uint16_t x1000Current);
};

var SSettings g_settings;

