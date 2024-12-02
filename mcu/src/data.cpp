// Define 'var' to instantiate variables
#define var

// Include the main include file to instantiate all defined variables
#include "includes.h"

uint16_t SSettings::AdcVoltageToDisplayX1000(uint16_t adcVoltage)
{
    // Vx1000 = (ADCv + m_voltageOffset)*m_voltage4096Value/4096
    int16_t voltage = static_cast<int16_t>(adcVoltage) + static_cast<int16_t>(m_voltageOffset);
    if (voltage < 0)
        voltage = 0;
    
    return utils::ShiftRight12(static_cast<uint32_t>(voltage)*m_voltage4096Value + 2048);
}

uint16_t SSettings::AdcCurrentToDisplayX1000(uint16_t adcCurrent)
{
    // Cx1000 = (ADCc + m_currentOffset)*m_current4096Value/4096
    int16_t current = static_cast<int16_t>(adcCurrent) + static_cast<int16_t>(m_currentOffset);
    if (current < 0)
        current = 0;

    return utils::ShiftRight12(static_cast<uint32_t>(current)*m_current4096Value + 2048);
}

uint16_t SSettings::DisplayX1000VoltageToAdc(uint16_t x1000Voltage)
{
    int16_t voltage = static_cast<int16_t>((utils::ShiftLeft12(x1000Voltage) + 18000)/m_voltage4096Value) -
        static_cast<int16_t>(m_voltageOffset);
    if (voltage < 0)
        voltage = 0;
    
    return static_cast<uint16_t>(voltage);
}

uint16_t SSettings::DisplayX1000CurrentToAdc(uint16_t x1000Current)
{
    int16_t current = static_cast<int16_t>((utils::ShiftLeft12(x1000Current) + 9000)/m_current4096Value) -
        static_cast<int16_t>(m_currentOffset);
    if (current < 0)
        current = 0;
    
    return static_cast<uint16_t>(current);
}

SSettings* SSettings::GetEepromSettingsAddr()
{
    return reinterpret_cast<SSettings*>(EEPROM_ADDR_SETTINGS);
}

bool SSettings::AreEepromSettingsValid()
{
    return eeprom_read_word(&GetEepromSettingsAddr()->m_magicNumber) == SSettings::MagicNumber;
}

bool SSettings::ReadFromEeprom()
{
    if (!AreEepromSettingsValid())
        return false;

    eeprom_read_block(this, GetEepromSettingsAddr(), sizeof(SSettings));
    return true;
}

void SSettings::SaveToEeprom()
{
    eeprom_update_block(this, GetEepromSettingsAddr(), sizeof(SSettings));
}

void SSettings::ResetToDefault()
{
    static const SSettings pm_defaultSettings PROGMEM = 
    {
        .m_keyBeepLength = 2,
        .m_keyBeepVolume = 8,

        .m_voltageOffset = DEFAULT_VOLTAGE_OFFSET,
        .m_voltage4096Value = DEFAULT_VOLTAGE_COEFF,
        .m_currentOffset = DEFAULT_CURRENT_OFFSET,
        .m_current4096Value = DEFAULT_CURRENT_COEFF,

        .m_musicVolume = 15,
        .m_chargeStartMusic = 1,
        .m_chargeEndMusic = 3,
        .m_chargeInterruptedMusic = 5,
        .m_badBatteryMusic = 7,

        .m_chargerProfileNumber = 0,
        .m_psSettings = {12000, 1000},

        .m_fanPwmMin = 7,
        .m_fanPwmMax = 70,
        .m_fanTempMin = 45,
        .m_fanTempMax = 60,
        .m_fanTempStop = 40,

        .m_fanPowMin = 100,
        .m_fanPowMax = 255,
        .m_fanPowStop = 85,

        .m_psProfiles =
        {
            { 5000, 6000},
            {12000, 6000},
            {21000, 6000},
            { 4200, 1500},
            { 8400, 1500},
            {12600, 1500},
            {16800, 1500},
            {21000, 1500},
            { 4200,  300},
            { 8400,  300},
        },
        .m_magicNumber = MagicNumber,
    };

    memcpy_P(this, &pm_defaultSettings, sizeof(SSettings));
}

bool SSettings::AreSettingsChanged()
{
    uint8_t* eepromAddr = reinterpret_cast<uint8_t*>(GetEepromSettingsAddr());
    uint8_t* dataAddr = reinterpret_cast<uint8_t*>(this);
    for (uint8_t i = 0; i < sizeof(SSettings); ++i, ++eepromAddr, ++dataAddr)
        if (*dataAddr != eeprom_read_byte(eepromAddr))
            return true;

    return false;
}

void SSettings::SetFanSpeed()
{
    constexpr uint8_t DONT_CHANGE = 1;
    uint8_t pwmMin = m_fanPwmMin;
    uint8_t pwmMax = m_fanPwmMax;
    uint16_t pwmDiff = static_cast<uint16_t>(pwmMax - pwmMin);

    // MOSFET and diode cooling
    const auto GetTempPwm = [&]() -> uint8_t
    {
        int8_t temperature = static_cast<int8_t>(g_temperatureBoard >> 8);
        if (temperature < m_fanTempStop)
            return 0;

        int8_t fanTempMin = m_fanTempMin;
        if (temperature < fanTempMin)
            return DONT_CHANGE;

        if (temperature >= m_fanTempMax)
            return pwmMax;

        // FanPwm = (FanMax - FanMin)*(t - tmin)/(tmax - tmin) + FanMin
        uint16_t value = pwmDiff;
        value *= static_cast<uint16_t>((g_temperatureBoard >> 5) - (fanTempMin << 3));
        value >>= 3;
        return static_cast<uint8_t>(value/(m_fanTempMax - fanTempMin)) + pwmMin;
    };

    // Choke cooling
    const auto GetCurrentPwm = [&]() -> uint8_t
    {
        cli();
        uint16_t current = g_adcCurrentAverage;
        sei();

        if (current < 4500/3)
            return 0;

        if (current < 4800/3)
            return DONT_CHANGE;

        return pwmMin;
    };

    // Power supply cooling
    const auto GetWattagePwm = [&]() -> uint8_t
    {
        cli();
        uint8_t voltage = g_adcVoltageAverage >> 8;
        uint8_t current = g_adcCurrentAverage >> 8;
        sei();

        // Wattage unit = ~1.18W, wattage range is [0..225]
        uint8_t wattage = voltage*current;
        if (wattage < m_fanPowStop)
            return 0;

        if (wattage < m_fanPowMin)
            return 0;

        if (wattage >= m_fanPowMax)
            return pwmMax;

        uint16_t value = pwmDiff;
        value *= static_cast<uint16_t>(wattage - m_fanPowMin);
        return static_cast<uint8_t>(value/(m_fanPowMax - m_fanPowMin)) + pwmMin;
    };

    uint8_t pwmValue = GetTempPwm();
    uint8_t pwmSecondary = GetCurrentPwm();
    if (pwmSecondary > pwmValue)
        pwmValue = pwmSecondary;

    pwmSecondary = GetWattagePwm();
    if (pwmSecondary > pwmValue)
        pwmValue = pwmSecondary;

    if (pwmValue != DONT_CHANGE)
        OCR0B = pwmValue;
}
