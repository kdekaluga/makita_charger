#pragma once

#include "data.h"

namespace charger {

enum class EState : uint8_t
{
    NO_BATTERY = 0,
    INVALID_BATTERY,
    INVALID_BATTERY2,
    MEASURING_VOLTAGE,
    CHARGING,
    CHARGE_COMPLETE,

    // Do nothing
    DO_NOTHING = 0x0E,

    // Don't change the current state, just reset the tick counter
    RESET_TICKS = 0x0F,

    STATE_MASK = 0x0F,

    // Don't erase background to prevent flickering
    DONT_ERASE_BACKGROUND = 0x80,
};

inline EState operator ~(EState op1)
{
    return static_cast<EState>(~static_cast<uint8_t>(op1));
}

inline EState operator &(EState op1, EState op2)
{
    return static_cast<EState>(static_cast<uint8_t>(op1) & static_cast<uint8_t>(op2));
}

inline EState operator |(EState op1, EState op2)
{
    return static_cast<EState>(static_cast<uint8_t>(op1) | static_cast<uint8_t>(op2));
}

struct SProfile
{
    // Profile name, up to 20 characters
    char m_name[20];

    // Profile name length
    uint8_t m_nameLength;

    // Target charge voltage and default charge current (current can be changed in the UI)
    uint16_t m_chargeVoltageX1000;
    uint16_t m_chargeCurrentX1000;

    // No-battery output voltage and current. It is used to pre-charge the output capacitors, so
    // when you connect a battery, the voltage difference is not so high to reduce sparkling.
    // The charger also detects battery presence if it switches to CC (battery voltage is lower)
    // or the measured voltage is 0.5 V higher.
    uint16_t m_openVoltageX1000;
    uint16_t m_openCurrentX1000;

    // Minimal valid battery voltage (maximum is m_chargeVoltageX1000), otherwise
    // an invalid battery error is displayed. Also used for charge estimation
    uint16_t m_minBatteryVoltageX1000;

    // If a battery was successfully charged, then left in the charger and its voltage
    // drops below this level, the charge is restarted.
    uint16_t m_restartChargeVoltageX1000;

    // When the charge current drops below this percent of its initial value, the charge
    // is considered complete (for the CC/CV mode).
    uint8_t m_stopChargeCurrentPercent;

    // Whether to use Makita 3rd pin to detect battery issues
    bool m_use3rdPin;

    // Whether to use continuos constant current mode
    bool m_cccMode;
};

} // namespace charger
