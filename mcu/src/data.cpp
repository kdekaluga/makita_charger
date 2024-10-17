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
    
    return utils::ShiftRight12(static_cast<uint32_t>(voltage)*m_voltage4096Value);
}

uint16_t SSettings::AdcCurrentToDisplayX1000(uint16_t adcCurrent)
{
    // Cx1000 = (ADCc + m_currentOffset)*m_current4096Value/4096
    int16_t current = static_cast<int16_t>(adcCurrent) + static_cast<int16_t>(m_currentOffset);
    if (current < 0)
        current = 0;

    return utils::ShiftRight12(static_cast<uint32_t>(current)*m_current4096Value);
}

uint16_t SSettings::DisplayX1000VoltageToAdc(uint16_t x1000Voltage)
{
    int16_t voltage = static_cast<int16_t>(utils::ShiftLeft12(x1000Voltage)/m_voltage4096Value) -
        static_cast<int16_t>(m_voltageOffset);
    if (voltage < 0)
        voltage = 0;
    
    return static_cast<uint16_t>(voltage);
}

uint16_t SSettings::DisplayX1000CurrentToAdc(uint16_t x1000Current)
{
    int16_t current = static_cast<int16_t>(utils::ShiftLeft12(x1000Current)/m_current4096Value) -
        static_cast<int16_t>(m_currentOffset);
    if (current < 0)
        current = 0;
    
    return static_cast<uint16_t>(current);
}
