#pragma once

// 'var' should be defined to the empty string in one cpp unit to instantiate variables.
#ifndef var
#define var extern
#endif

#define EEPROM_ADDR_SETTINGS 0x0000
#define EEPROM_ADDR_PROFILES 0x0200
#define EEPROM_PROFILES_COUNT 10

#include <stdint.h>

// General purpose buffer
var char g_buffer[16];

// Timer0 overflow interrupt counter, 0x00, 0x20, 0x40, ... 0xE0
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

// Last encoder pin state (for the main interrupt)
var uint8_t g_encoderPinState;

// Encoder cycle (0, 4, 8, 12)
var uint8_t g_encoderCycle;

// Represents relative encoder position
var int8_t g_encoderCounter;

enum class EEncoderKey : uint8_t
{
    None = 0,
    Down,
    Up,
    DownLong,
    UpLong,
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

// State of the temperature sensor request routine
var uint8_t g_tempRequesterState;

// Board and battery temprerature
var uint16_t g_temperatureBoard;
var uint16_t g_temperatureBattery;

// ***

struct SPsProfile
{
    uint16_t m_voltage;
    uint16_t m_current;
};

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

    // Music player volume
    uint8_t m_musicVolume;

    // Charge musics
    uint8_t m_chargeStartMusic;
    uint8_t m_chargeEndMusic;
    uint8_t m_chargeInterruptedMusic;
    uint8_t m_badBatteryMusic;
    uint8_t m_batteryErrorMusic;

    // Current charger profile number
    uint8_t m_chargerProfileNumber;

    // Voltage and current settings for the power supply mode
    SPsProfile m_psSettings;

    // Fan settings
    // Minimum and maximum allowed fan PWM values (OCR0B values)
    uint8_t m_fanPwmMin;
    uint8_t m_fanPwmMax;
    
    // Fan start, maximum and stop temperatures
    int8_t m_fanTempMin;
    int8_t m_fanTempMax;
    int8_t m_fanTempStop;

    // Fan start, max and stop wattage (in 1.18W units)
    uint8_t m_fanPowMin;
    uint8_t m_fanPowMax;
    uint8_t m_fanPowStop;

    // Additional power supply profiles (10 pcs)
    SPsProfile m_psProfiles[10];

    // Magic number
    static constexpr uint16_t MagicNumber = 0x1236;
    uint16_t m_magicNumber;

    // Measurement conversion routines
    uint16_t AdcVoltageToDisplayX1000(uint16_t adcVoltage);
    uint16_t AdcCurrentToDisplayX1000(uint16_t adcCurrent);
    uint16_t DisplayX1000VoltageToAdc(uint16_t x1000Voltage);
    uint16_t DisplayX1000CurrentToAdc(uint16_t x1000Current);

    static bool AreEepromSettingsValid();
    bool ReadFromEeprom();
    void SaveToEeprom();
    void ResetToDefault();
    bool AreSettingsChanged();

    void SetFanSpeed();

private:
    static SSettings* GetEepromSettingsAddr();
};

var SSettings g_settings;

