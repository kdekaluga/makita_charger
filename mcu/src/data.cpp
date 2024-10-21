// Define 'var' to instantiate variables
#define var

// Include the main include file to instantiate all defined variables
#include "includes.h"

#include <avr/eeprom.h>

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
        .m_voltageOffset = 15,
        .m_voltage4096Value = 24532,
        .m_currentOffset = 0,
        .m_current4096Value = 12580,
        .m_psSettings = {12000, 1000},
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

