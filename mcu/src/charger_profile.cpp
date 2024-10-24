#include "includes.h"

// Uncomment the following lines to get a compilation error with the charger profile size
template<size_t s> class PrintSize;
//PrintSize<sizeof(::charger::SProfile)> printSize;


namespace charger {

static const SProfile pm_profiles[EEPROM_PROFILES_COUNT] PROGMEM =
{
    // Profile 0
    {
        .m_name = {_DRO_STR20("Makita 18V 3pin     ")},
        .m_nameLength = 15,
        .m_chargeVoltageX1000 = 21000,
        .m_chargeCurrentX1000 = 2000,
        .m_openVoltageX1000 = 21500,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 15000,
        .m_restartChargeVoltageX1000 = 20500,
        .m_stopChargeCurrentPercent = 10,
        .m_options = COPT_USE_3RD_PIN,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 1
    {
        .m_name = {_DRO_STR20("Makita 18V 2pin     ")},
        .m_nameLength = 15,
        .m_chargeVoltageX1000 = 21000,
        .m_chargeCurrentX1000 = 2000,
        .m_openVoltageX1000 = 21500,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 15000,
        .m_restartChargeVoltageX1000 = 20500,
        .m_stopChargeCurrentPercent = 10,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 2
    {
        .m_name = {_DRO_STR20("Li-Ion 1S 4.2V 1.5A ")},
        .m_nameLength = 19,
        .m_chargeVoltageX1000 = 4200,
        .m_chargeCurrentX1000 = 1500,
        .m_openVoltageX1000 = 4700,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 3000,
        .m_restartChargeVoltageX1000 = 4100,
        .m_stopChargeCurrentPercent = 5,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 3
    {
        .m_name = {_DRO_STR20("Li-Ion 2S 8.4V 1.5A ")},
        .m_nameLength = 19,
        .m_chargeVoltageX1000 = 8400,
        .m_chargeCurrentX1000 = 1500,
        .m_openVoltageX1000 = 8900,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 6000,
        .m_restartChargeVoltageX1000 = 8200,
        .m_stopChargeCurrentPercent = 5,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 4
    {
        .m_name = {_DRO_STR20("Li-Ion 3S 12.6V 1.5A")},
        .m_nameLength = 20,
        .m_chargeVoltageX1000 = 12600,
        .m_chargeCurrentX1000 = 1500,
        .m_openVoltageX1000 = 23100,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 9000,
        .m_restartChargeVoltageX1000 = 12300,
        .m_stopChargeCurrentPercent = 5,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 5
    {
        .m_name = {_DRO_STR20("Li-Ion 4S 16.8V 1.5A")},
        .m_nameLength = 20,
        .m_chargeVoltageX1000 = 16800,
        .m_chargeCurrentX1000 = 1500,
        .m_openVoltageX1000 = 17300,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 12000,
        .m_restartChargeVoltageX1000 = 16400,
        .m_stopChargeCurrentPercent = 5,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 6
    {
        .m_name = {_DRO_STR20("Li-Ion 5S 21.0V 1.5A")},
        .m_nameLength = 20,
        .m_chargeVoltageX1000 = 21000,
        .m_chargeCurrentX1000 = 1500,
        .m_openVoltageX1000 = 21500,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 15000,
        .m_restartChargeVoltageX1000 = 20500,
        .m_stopChargeCurrentPercent = 5,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 7
    {
        .m_name = {_DRO_STR20("Li 6F22 8.4V 0.3A   ")},
        .m_nameLength = 17,
        .m_chargeVoltageX1000 = 8400,
        .m_chargeCurrentX1000 = 300,
        .m_openVoltageX1000 = 8900,
        .m_openCurrentX1000 = 15,
        .m_minBatteryVoltageX1000 = 6000,
        .m_restartChargeVoltageX1000 = 8200,
        .m_stopChargeCurrentPercent = 5,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 8
    {
        .m_name = {_DRO_STR20("12V Pb Charge       ")},
        .m_nameLength = 13,
        .m_chargeVoltageX1000 = 14400,
        .m_chargeCurrentX1000 = 3000,
        .m_openVoltageX1000 = 14900,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 10000,
        .m_restartChargeVoltageX1000 = 10000,
        .m_stopChargeCurrentPercent = 5,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },

    // Profile 9
    {
        .m_name = {_DRO_STR20("12V Pb Repair       ")},
        .m_nameLength = 13,
        .m_chargeVoltageX1000 = 19000,
        .m_chargeCurrentX1000 = 2000,
        .m_openVoltageX1000 = 19500,
        .m_openCurrentX1000 = 30,
        .m_minBatteryVoltageX1000 = 10000,
        .m_restartChargeVoltageX1000 = 10000,
        .m_stopChargeCurrentPercent = 5,
        .m_options = 0,
        .m_magicNumber = SProfile::MagicNumber,
    },
};

//PrintSize<sizeof(pm_profiles)> printSize;

void SProfile::LoadFromEeprom(uint8_t nProfile)
{
    if (nProfile >= EEPROM_PROFILES_COUNT)
        nProfile = 0;

    // Check whether we have a valid profile in EEPROM
    SProfile* eepromProfile = GetProfileEepromAddr(nProfile);
    if (eeprom_read_byte(&eepromProfile->m_magicNumber) == MagicNumber)
        eeprom_read_block(this, eepromProfile, sizeof(SProfile));
    else
        memcpy_P(this, &pm_profiles[nProfile], sizeof(SProfile));
}

void SProfile::SaveToEeprom(uint8_t nProfile)
{
    if (nProfile >= EEPROM_PROFILES_COUNT)
        nProfile = 0;

    eeprom_update_block(this, GetProfileEepromAddr(nProfile), sizeof(SProfile));
}

SProfile* SProfile::GetProfileEepromAddr(uint8_t nProfile)
{
    return reinterpret_cast<SProfile*>(EEPROM_ADDR_PROFILES + nProfile*sizeof(SProfile));
}

} // namespace charger