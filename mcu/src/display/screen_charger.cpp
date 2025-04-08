#include "../includes.h"

namespace screen::charger {

using ::charger::g_profile;
using ::charger::g_tempProfile;

#define CHARGE_BAR_WIDTH 7

#define UI_ELEMENT_COUNT 7

#define UI_CCCMODE 0
#define UI_CURRENT1 1
#define UI_CURRENT2 2
#define UI_CURRENT3 3
#define UI_OPT_MAKITA_PROTO 4
#define UI_OPT_CHARGE_RESTART 5
#define UI_BATTERY_ERROR_CONTINUE 6

constexpr uint8_t ChargeModeYPos = 54;
constexpr uint8_t CurrentYPos = 81;
constexpr uint8_t ChargeOptionsYPos = 108;

static const char pm_cmTitle[] PROGMEM = "Select profile";
static const char pm_cmReturn[] PROGMEM = "Return";
static const char pm_cmExit[] PROGMEM = "Exit Charger";
static const char pm_cmEditParams[] PROGMEM = "Charge parameters";

uint8_t CmDrawItem(uint8_t x, uint8_t y, uint8_t nItem)
{
    if (nItem == 0)
        return display::PrintString(x, y, pm_cmReturn);

    if (nItem == 1)
        return display::PrintString(x, y, pm_cmExit);

    if (nItem == 2)
        return display::PrintString(x, y, pm_cmEditParams);

    g_tempProfile.LoadFromEeprom(nItem - 3);
    return display::PrintStringRam(x, y, g_tempProfile.m_name, g_tempProfile.m_nameLength);
}

uint8_t CmGetItemWidth(uint8_t nItem)
{
    if (nItem == 0)
        return display::GetTextWidth(pm_cmReturn);
    
    if (nItem == 1)
        return display::GetTextWidth(pm_cmExit);

    if (nItem == 2)
        return display::GetTextWidth(pm_cmEditParams);

    g_tempProfile.LoadFromEeprom(nItem - 3);
    return display::GetTextWidthRam(g_tempProfile.m_name, g_tempProfile.m_nameLength);    
}

static const display::Menu pm_chargerMenu PROGMEM =
{
    nullptr,
    &CmDrawItem,
    &CmGetItemWidth,
    EEPROM_PROFILES_COUNT + 3,
    pm_cmTitle
};

uint16_t SmoothValue(uint16_t value, uint16_t& prevValue, int8_t& trend)
{
    int16_t dv = static_cast<int16_t>(value - prevValue);
    if (trend >= 0)
    {
        if (dv <= -18)
        {
            trend = -1;
            prevValue = value;
        }
        else if (dv >= -5)
        {
            trend = 1;
            if (dv >= 5)
                prevValue = value;
        }
        else if (++trend >= 10)
        {
            trend = 1;
            prevValue = value;
        }
    }
    else
    {
        if (dv >= 18)
        {
            trend = 1;
            prevValue = value;
        }
        else if (dv <= 5)
        {
            trend = -1;
            if (dv <= -5)
                prevValue = value;
        }
        else if (--trend <= -10)
        {
            trend = -1;
            prevValue = value;
        }
    }

    return prevValue;
}

void SetNoBatteryOuputValues()
{
    g_pidTargetVoltage = g_settings.DisplayX1000VoltageToAdc(g_profile.m_openVoltageX1000);

    // We may have a positive current offset after calibration. This means that when ADC reports zero current,
    // we are displaying some non-zero value and there is no way for us to register current lower than this.
    // In this case we can't just set a very small output current since it could be lower than the offset value
    // and thus PID will always be in the CC mode. To avoid this we add calibrated current offset to the open
    // current setting.
    uint16_t correction = g_settings.AdcCurrentToDisplayX1000(0);
    g_openCurrentCorrected = g_profile.m_openCurrentX1000 + correction;
    g_pidTargetCurrent = g_settings.DisplayX1000CurrentToAdc(g_openCurrentCorrected);
    g_batteryChargePercent = g_batteryChargePixels = 0;
    g_outOn = true;
}

void SetWorkingOutputValues()
{
    g_chargeFinishCurrentThreshold = static_cast<uint16_t>(
        (static_cast<uint32_t>(g_profile.m_chargeCurrentX1000)*g_profile.m_stopChargeCurrentPercent)/100
    );

    g_noBatteryThresholdCurrent = g_openCurrentCorrected;
    if (g_noBatteryThresholdCurrent + 10 >= g_chargeFinishCurrentThreshold)
        g_noBatteryThresholdCurrent = g_chargeFinishCurrentThreshold - 10;

    g_pidTargetVoltage = g_settings.DisplayX1000VoltageToAdc(
        (g_profile.m_options & COPT_CCC_MODE) ? 24000 : g_profile.m_chargeVoltageX1000);
    g_pidTargetCurrent = g_settings.DisplayX1000CurrentToAdc(g_profile.m_chargeCurrentX1000);
} 

void Init()
{
    SetNoBatteryOuputValues();

    g_state = EState::NO_BATTERY;
    g_ticksInState = 0;
}

int8_t DrawBackground()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(61, 6 + 17, S, "CHARGER", 7),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 2, 32, 236, 178,
        
        DRO_BGCOLOR(CLR_BLUE),
        DRO_FILLRECT | 6,
            0, 30, 2, 180,
            238, 30, 2, 180,
            2, 30, 236, 2,
            91, 32, 2, 83,
            2, 115, 236, 2,
            0, 210, 240, 30,

        DRO_BGCOLOR(CLR_DARK_BLUE),
        DRO_FILLRECT | 1, 2, 32, 89, 83,
        DRO_STR(17 + 13*3 + 6, CurrentYPos, S, "A", 1),
        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);
    display::SetSans12();
    display::SetColors(CLR_BLUE, CLR_WHITE);
    uint8_t width = display::GetTextWidthRam(g_profile.m_name, g_profile.m_nameLength);
    display::PrintStringRam((240 - width)/2, 233, g_profile.m_name, g_profile.m_nameLength);
    display::SetBgColor(CLR_BLACK);

    return DSD_CURSOR_HIDDEN;
}

void EraseBackground()
{
    static const display::Rect pm_eraseBgRects[] PROGMEM =
    {
        {93, 32, 145, 83},
        {2, 117, 236, 67},
    };
    display::FillRects(pm_eraseBgRects, 2, CLR_BLACK);
}

EState StateMachine(EState state, uint16_t voltage, uint16_t current)
{
    uint16_t ticksInState = g_ticksInState;

    const auto StartCharge = [&]() -> EState
    {
        utils::TimeCapacityReset();
        SetWorkingOutputValues();
        g_batteryChargeBarPosition = -CHARGE_BAR_WIDTH;
        sound::PlayMusic(g_settings.m_chargeStartMusic);
        return EState::MEASURING_VOLTAGE;
    };

    const auto FinishCharge = [&]() -> EState
    {
        sound::PlayMusic(g_settings.m_chargeEndMusic);
        return EState::CHARGE_COMPLETE;
    };

    const auto NoBatteryMakita = [&]() -> EState
    {
        // Perform 1-wire reset five times and make sure the battery always responds
        for (uint8_t i = 0; i < 5; ++i)
        {
            utils::Delay(10);

            if (!one_wire::Reset())
                return EState::RESET_TICKS;
        }

        g_outOn = false;

        // OK, switch battery to the charge mode
        one_wire::Send(0xCC);
        one_wire::Send(0xF0);
        one_wire::Send(0x00);

        // Read the battery message
        for (uint8_t i = 0; i < 32; ++i)
            g_batteryMessage[i] = one_wire::Recv();

        // Wait a little bit
        utils::Delay(50);

        return StartCharge();
    };

    const auto BatteryError = [&]() -> EState
    {
        g_outOn = false;
        sound::PlayMusic(g_settings.m_batteryErrorMusic);
        return EState::BATTERY_ERROR;
    };

    switch (state)
    {
    case EState::NO_BATTERY:
        if (g_ticksInState < 10)
            return EState::DO_NOTHING;

        if (g_profile.m_options & COPT_MAKITA_PROTOCOL)
            return NoBatteryMakita();

        // Wait until the output voltage is quite stable
        if (ABS(static_cast<int16_t>(voltage - g_previousBatteryVoltage)) > 100)
        {
            g_previousBatteryVoltage = voltage;
            return EState::RESET_TICKS;
        }

        g_previousBatteryVoltage = voltage;

        // Wait until we detect something
        if (g_pidMode != PID_MODE_CC && static_cast<int16_t>(voltage - g_profile.m_openVoltageX1000) < 200)
            return EState::RESET_TICKS;

        // We must be detecting something for at least 500 ms
        if (g_ticksInState < 50)
            return EState::DO_NOTHING;
        
        // Something detected, but it could either be a valid or invalid battery. Switch off the output
        g_outOn = false;
        if (voltage > g_profile.m_chargeVoltageX1000 || voltage < g_profile.m_minBatteryVoltageX1000)
        {
            sound::PlayMusic(g_settings.m_badBatteryMusic);
            return EState::INVALID_BATTERY;
        }

        // Battery is OK, start charging process with the voltage measurement
        return StartCharge();

    case EState::INVALID_BATTERY:
        // Wait until either the voltage drops by 300 mV or drops below 200 mV.
        // This would mean that either we have a short circuit or the invalid battery has been removed.
        if (voltage > 200 && g_previousBatteryVoltage - voltage < 300)
            return EState::RESET_TICKS;

        // Wait 1000 ms more
        if (ticksInState < 100)
            return EState::DO_NOTHING;

        // Switch output on
        g_outOn = true;
        return EState::INVALID_BATTERY2 | EState::DONT_ERASE_BACKGROUND;

    case EState::INVALID_BATTERY2:
        // Wait for output to reach the target voltage. This will protect us from a short circuit
        if (ABS(static_cast<int16_t>(voltage - g_profile.m_openVoltageX1000)) > 100)
            return EState::RESET_TICKS;

        // Wait 200 ms more
        if (ticksInState < 20)
            return EState::DO_NOTHING;

        return EState::NO_BATTERY;

    case EState::MEASURING_VOLTAGE:
        // First, wait 100 ms after switching off the output
        if (ticksInState < 10)
            return EState::DO_NOTHING;

        // Estimate the current battery charge in percents and pixels (to draw the battery icon) using
        // the following formula:
        // Charge% = (voltage - minVoltage)/(maxVoltage - maxVoltage)*100
        if (voltage >= g_profile.m_minBatteryVoltageX1000)
        {
            uint16_t maxDv = g_profile.m_chargeVoltageX1000 - g_profile.m_minBatteryVoltageX1000;
            uint32_t dv = voltage - g_profile.m_minBatteryVoltageX1000;
            g_batteryChargePercent = static_cast<uint8_t>(dv*100/maxDv);
            g_batteryChargePixels = static_cast<uint8_t>(dv*78/maxDv);
            if (g_batteryChargePercent > 100)
            {
                g_batteryChargePercent = 100;
                g_batteryChargePixels = 78;
            }
        }
        else
        {
            g_batteryChargePercent = 0;
            g_batteryChargePixels = 0;
        }

        // If we're in the CCC mode and we've reached our target voltage, stop the charge
        if ((g_profile.m_options & COPT_CCC_MODE) && voltage >= g_profile.m_chargeVoltageX1000)
            return FinishCharge();

        // Switch output back on
        g_outOn = true;
        g_chargeCanBeFinished = true;
        g_noBatteryDetectCount = 0;
        return EState::CHARGING | EState::DONT_ERASE_BACKGROUND;

    case EState::CHARGING:
        // Don't do anything in the first 100 ms
        if (ticksInState < 10)
            return EState::DO_NOTHING;

        // Once in 10 seconds check whether we've charged the battery and measure the battery voltage
        if (ticksInState >= 1000)
        {
            g_outOn = false;
            if ((g_profile.m_options & COPT_CCC_MODE) || !g_chargeCanBeFinished)
                return EState::MEASURING_VOLTAGE | EState::DONT_ERASE_BACKGROUND;

            return FinishCharge();            
        }

        // If the charge current exceeds the threshold value at least once in 10 seconds,
        // the charge is not finished yet
        if (current >= g_chargeFinishCurrentThreshold)
            g_chargeCanBeFinished = false;

        // Check battery status
        if (g_profile.m_options & COPT_MAKITA_PROTOCOL)
        {
            if (PINB & BV(PB_IN_BATTERY_STATUS))
            {
                g_noBatteryDetectCount = 0;
                return EState::DO_NOTHING;
            }

            if (++g_noBatteryDetectCount < 3)
                return EState::DO_NOTHING;

            // The battery is here but its status line is low
            if (one_wire::Reset())
                return BatteryError();

        } else
        {
            // Battery was removed?
            if (current >= g_noBatteryThresholdCurrent)
            {
                g_noBatteryDetectCount = 0;
                return EState::DO_NOTHING;
            }

            if (++g_noBatteryDetectCount < 3)
                return EState::DO_NOTHING;
        }

        SetNoBatteryOuputValues();
        sound::PlayMusic(g_settings.m_chargeInterruptedMusic);
        return EState::NO_BATTERY;

    case EState::CHARGE_COMPLETE:
        // Wait until voltage drops below the charge restart level
        if (voltage >= g_profile.m_restartChargeVoltageX1000)
        {
            g_previousBatteryVoltage = voltage;
            return EState::RESET_TICKS;
        }
        
        // Wait 1.5 seconds more
        if (ticksInState < 150)
            return EState::DO_NOTHING;

        // If the battery voltage drops quite fast (more than 100 mV in 1.5 s),
        // we consider that a battery was removed
        if (static_cast<int16_t>(g_previousBatteryVoltage - voltage) > 100)
        {
            SetNoBatteryOuputValues();
            sound::StopMusic();
            return EState::NO_BATTERY;
        }

        // Either restart the charge (if this option is on) or repeat the check again
        if (g_profile.m_options & COPT_RESTART_CHARGE)
        {
            sound::PlayMusic(g_settings.m_chargeStartMusic);
            return EState::MEASURING_VOLTAGE;
        }

        g_previousBatteryVoltage = voltage;
        return EState::RESET_TICKS;

    case EState::BATTERY_ERROR:
        return EState::DO_NOTHING;

    default:
        Init();
        return EState::NO_BATTERY;
    }
}

void DrawBattery (int8_t chargeBarPos)
{
    static const display::Rect pm_batteryRects[] PROGMEM =
    {
        {123, 43, 84, 2},
        {123, 76, 84, 2},
        {122, 44, 2, 33},
        {206, 44, 2, 33},
        {208, 49, 1, 23},
        {209, 55, 3, 11},
    };
    display::FillRects(pm_batteryRects, 6, CLR_GRAY);

    constexpr uint8_t cbWidth = CHARGE_BAR_WIDTH;
    constexpr uint8_t x = 126;
    constexpr uint8_t y = 47;
    constexpr uint8_t h = 27;
    constexpr uint16_t color = RGB(0, 192, 0);

    uint8_t width = g_batteryChargePixels;
    if (chargeBarPos <= 0)
    {
        uint8_t w = chargeBarPos + cbWidth;
        if (w > width)
            w = width;

        display::FillRect(x, y, w, h, CLR_WHITE);
        display::FillRect(x + w, y, width - w, h, color);
    }
    else if (chargeBarPos < static_cast<int8_t>(width - cbWidth))
    {
        display::FillRect(x, y, chargeBarPos, h, color);
        display::FillRect(x + chargeBarPos, y, cbWidth, h, CLR_WHITE);
        display::FillRect(x + chargeBarPos + cbWidth, y, width - chargeBarPos - cbWidth, h, color);
    }
    else if (chargeBarPos < static_cast<int8_t>(width))
    {
        display::FillRect(x, y, chargeBarPos, h, color);
        display::FillRect(x + chargeBarPos, y, width - chargeBarPos, h, CLR_WHITE);
    }
    else
    {
        display::FillRect(x, y, width, h, color);
    }

    display::FillRect(x + width, y, 78 - width, h, CLR_BLACK);

    display::SetColor(CLR_WHITE);
    display::SetSans12();
    utils::PercentToString(g_batteryChargePercent);
    display::PrintStringRam(102, 106, g_buffer, 4);

    cli();
    uint16_t tempBattery = g_temperatureBattery;
    sei();

    display::SetColor(utils::GetBatteryTempColor(tempBattery));
    utils::TemperatureToString(utils::TemperatureToDisplayX100(tempBattery));
    display::PrintStringRam(174, 106, g_buffer + 1, 5);
};

void DrawElements(int8_t cursorPosition, uint8_t ticksElapsed)
{
    g_ticksInState += static_cast<uint16_t>(ticksElapsed);

    cli();
    uint16_t voltage = g_adcVoltageAverage;
    uint16_t current = g_adcCurrentAverage;
    int16_t tempBoard = g_temperatureBoard;
    sei();

    voltage = g_settings.AdcVoltageToDisplayX1000(voltage);
    current = g_settings.AdcCurrentToDisplayX1000(current);

    EState state = g_state;
    EState newState = StateMachine(state, voltage, current);
    if (newState != EState::DO_NOTHING)
    {
        if (newState != EState::RESET_TICKS)
        {
            if ((newState & EState::DONT_ERASE_BACKGROUND) != EState::DONT_ERASE_BACKGROUND)
                EraseBackground();

            g_state = state = newState & EState::STATE_MASK;
        }

        g_ticksInState = 0;
    }

    display::SetBgColor(CLR_BLACK);

    if (state == EState::NO_BATTERY)
    {
        static const uint8_t pm_noBatteryObjects[] PROGMEM =
        {
            DRO_STR(150, 67, S, "NO", 2),
            DRO_STR(114, 95, S, "BATTERY", 7),
            DRO_FGCOLOR(RGB(128, 128, 128)),
            DRO_STR(58, 143, S, "Insert battery", 14),
            DRO_STR(39, 170, S, "to start charging", 17),
            DRO_END
        };
        display::DrawObjects(pm_noBatteryObjects, CLR_BLACK, CLR_WHITE);
    }

    else if (state == EState::INVALID_BATTERY || state == EState::INVALID_BATTERY2)
    {
        static const uint8_t pm_invalidBatteryObjects[] PROGMEM =
        {
            DRO_STR(122, 67, S, "INVALID", 7),
            DRO_STR(110, 95, S, "BATTERY!", 8),
            DRO_FGCOLOR(RGB(128, 128, 128)),
            DRO_STR(41, 141, S, "Battery voltage:", 16),
            DRO_END
        };

        voltage = SmoothValue(voltage, g_smoothVoltageValue, g_smoothVoltageTrend);

        display::DrawObjects(pm_invalidBatteryObjects, CLR_BLACK, RGB(255, 153, 54));
        display::SetColor(RGB(255, 153, 54));
        display::SetSans18();
        utils::VoltageToString(voltage, true);
        display::PrintStringRam(68, 176, g_buffer, 6);
    }

    else if (state == EState::MEASURING_VOLTAGE || state == EState::CHARGING)
    {
        DrawBattery(HIBYTE(g_batteryChargeBarPosition));

        if (state == EState::CHARGING && g_ticksInState >= 20)
        {
            voltage = SmoothValue(voltage, g_smoothVoltageValue, g_smoothVoltageTrend);
            current = SmoothValue(current, g_smoothCurrentValue, g_smoothCurrentTrend);

            display::SetSans18();
            display::SetColor(CLR_VOLTAGE);
            utils::VoltageToString(voltage, true);
            display::PrintStringRam(10, 150, g_buffer, 6);

            // Width = 19*3 + 9 + 23 = 89 px
            display::SetColor(CLR_CURRENT);
            utils::CurrentToString(current);
            display::PrintStringRam(141, 150, g_buffer, 5);

            display::SetSans12();
            display::SetColor(CLR_WHITE);
            utils::WattageToString(voltage, current);
            display::PrintStringRam(10, 177, g_buffer, 6);

            // Width = 13*5 + 6 + 16 + 13 = 100 px
            utils::CapacityToString();
            display::PrintStringRam(130, 177, g_buffer, 8);
        }
    }

    else if (state == EState::CHARGE_COMPLETE)
    {
        DrawBattery(100);
        static const char pm_chargeComplete[] PROGMEM = "Charge complete";
        display::SetColor(CLR_GRAY);
        display::PrintString(32, 141, pm_chargeComplete);

        display::SetColor(RGB(128, 255, 0));
        display::SetSans18();
        utils::CapacityToString();
        display::PrintStringRam(47, 176, g_buffer, 8);
    }

    else if (state == EState::BATTERY_ERROR)
    {
        static const uint8_t pm_batteryErrorObjects[] PROGMEM =
        {
            DRO_STR(113, 67, S, "BATTERY", 7),
            DRO_STR(120, 95, S, "ERROR!", 6),
            DRO_FGCOLOR(CLR_GRAY),
            DRO_STR(10, 145, S, "V:", 2),
            DRO_STR(10, 173, S, "T:", 2),
            DRO_END
        };
        display::DrawObjects(pm_batteryErrorObjects, CLR_BLACK, RGB(255, 153, 54));

        cli();
        uint16_t tempBattery = g_temperatureBattery;
        sei();

        display::SetColor(utils::GetBatteryTempColor(tempBattery));
        utils::TemperatureToString(utils::TemperatureToDisplayX100(tempBattery));
        display::PrintStringRam(10 + 15 + 6 + 6, 173, g_buffer + 1, 5);

        display::SetColor(CLR_WHITE);
        utils::CapacityToString();
        display::PrintStringRam(130, 173, g_buffer, 8);

        voltage = SmoothValue(voltage, g_smoothVoltageValue, g_smoothVoltageTrend);
        display::SetColor(CLR_VOLTAGE);
        utils::VoltageToString(voltage, true);
        display::PrintStringRam(10 + 15 + 6 + 6, 145, g_buffer, 6);

        display::SetUiElementColors(cursorPosition, UI_BATTERY_ERROR_CONTINUE);
        static const char pm_continue[] PROGMEM = "Continue";
        display::PrintString(137, 145, pm_continue);
    }

    // Board temperature
    display::SetSans12();
    display::SetColors(CLR_BLACK, utils::GetBoardTempColor(tempBoard));
    utils::TemperatureToString(utils::TemperatureToDisplayX100(tempBoard));
    g_buffer[0] = TEMP_BOARD_SYMBOL;
    display::PrintStringRam(10, 203, g_buffer, 6);

    // Battery error flag
    if (state == EState::BATTERY_ERROR && (g_profile.m_options & COPT_MAKITA_PROTOCOL) &&
        (!(PINB & BV(PB_IN_BATTERY_STATUS))))
    {
        display::SetColor(CLR_RED_BEAUTIFUL);
        static const char pm_fail[] PROGMEM = "Err"; // 15 + 8 + 8 = 31
        display::PrintString(99, 203, pm_fail);
    }
    else
    {
        display::FillRect(99, 203 - 21, 31, 27, CLR_BLACK);
    }

    // Time
    // Width = 13*6 + 6*2 = 90 px
    display::SetColor(CLR_WHITE);
    utils::TimeToString();
    display::PrintStringRam(140, 203, g_buffer, 8);

    // Set current
    utils::CurrentToString(g_profile.m_chargeCurrentX1000);
    display::DrawSettableDecimal(17, CurrentYPos, 4, cursorPosition - UI_CURRENT1, CLR_WHITE, CLR_DARK_BLUE);

    // Charge mode
    display::SetBgColor(cursorPosition == UI_CCCMODE ? CLR_BG_CURSOR : CLR_DARK_BLUE);
    if (g_profile.m_options & COPT_CCC_MODE)
    {
        // Width = 17*3 = 51 px
        static const char pm_cccMode[] PROGMEM = "CCC";
        static const display::Rect pm_cccRects[] PROGMEM =
        {
            {10, ChargeModeYPos - 21, 11, 27},
            {10 + 11 + 51, ChargeModeYPos - 21, 11, 27},
        };
        display::FillRects(pm_cccRects, 2, display::g_bgColor);
        display::PrintString(10 + 11, ChargeModeYPos, pm_cccMode);
    }
    else
    {
        // Width = 17*3 + 7 + 15 = 73 px
        static const char pm_cccvMode[] PROGMEM = "CC/CV";
        display::PrintString(10, ChargeModeYPos, pm_cccvMode);
    }

    const auto DrawOption = [&](uint8_t x, uint8_t option, uint8_t uiPosition, const char* text)
    {
        display::SetBgColor(cursorPosition == uiPosition ? CLR_BG_CURSOR : CLR_DARK_BLUE);
        display::SetColor((g_profile.m_options & option) ? CLR_WHITE : RGB(192, 0, 0));
        display::PrintString(x, ChargeOptionsYPos, text);
    };

    // Options
    static const char pm_opt3Pin[] PROGMEM = "Mk";
    static const char pm_optRestart[] PROGMEM = "Rst";
    DrawOption(10, COPT_MAKITA_PROTOCOL, UI_OPT_MAKITA_PROTO, pm_opt3Pin);
    DrawOption(50, COPT_RESTART_CHARGE, UI_OPT_CHARGE_RESTART, pm_optRestart);

    g_batteryChargeBarPosition += static_cast<int16_t>(ticksElapsed) << 5;
    int8_t* chargeBarPos = reinterpret_cast<int8_t*>(&g_batteryChargeBarPosition) + 1;
    if (*chargeBarPos >= static_cast<int8_t>(g_batteryChargePixels))
        *chargeBarPos -= g_batteryChargePixels + CHARGE_BAR_WIDTH;

} // void DrawElements(int8_t cursorPosition, uint8_t ticksElapsed)

bool OnClick(int8_t cursorPosition)
{
    if (cursorPosition == UI_CCCMODE)
    {
        g_profile.m_options ^= COPT_CCC_MODE;
        if (g_state == EState::MEASURING_VOLTAGE || g_state == EState::CHARGING)
            SetWorkingOutputValues();

        return false;
    }

    if (cursorPosition == UI_OPT_MAKITA_PROTO)
    {
        // Makita protocol can be switched on or off only when there is no battery inserted
        if (g_state == EState::NO_BATTERY)
            g_profile.m_options ^= COPT_MAKITA_PROTOCOL;
        
        return false;
    }

    if (cursorPosition == UI_OPT_CHARGE_RESTART)
    {
        g_profile.m_options ^= COPT_RESTART_CHARGE;
        return false;
    }

    if (cursorPosition == UI_BATTERY_ERROR_CONTINUE)
    {
        if (g_state == EState::BATTERY_ERROR)
        {
            EraseBackground();
            g_state = EState::MEASURING_VOLTAGE;
            g_ticksInState = 0;
        }

        return false;
    }

    return true;
}

void OnChangeValue(int8_t cursorPosition, int8_t delta)
{
    if (cursorPosition >= UI_CURRENT1 && cursorPosition <= UI_CURRENT3)
    {
        g_profile.m_chargeCurrentX1000 = utils::ChangeI16ByDigit(
            g_profile.m_chargeCurrentX1000,
            UI_CURRENT3 + 1 - cursorPosition,
            delta,
            g_profile.m_openCurrentX1000 + 10,
            MAX_CURRENT
        );

        if (g_state == EState::MEASURING_VOLTAGE || g_state == EState::CHARGING)
            SetWorkingOutputValues();
    }
}

bool OnLongClick(int8_t cursorPosition)
{
    static const char pm_stopTitle[] PROGMEM = "Stop charge";
    static const char pm_stopText[] PROGMEM =
        "This will stop the\ncharge process.\nAre you sure want\nto continue?";

    // Switch off the output. This is very important since we could be in the
    // CCC mode (where the output voltage is huge and we must periodically switch it off)
    // and the user may left the message box/menu unattended forever.
    g_outOn = false;

    // If we're charging, ask the user whether they want to continue
    if (g_state == EState::MEASURING_VOLTAGE || g_state == EState::CHARGING)
    {
        if (display::MessageBox(pm_stopTitle, pm_stopText, MB_YESNO | MB_INFO | MB_DEFAULT_NO) != 0)
        {
            DrawBackground();

            // Switch the output back on and reset the tick counter,
            // so no action will be performed right now
            g_outOn = true;
            g_ticksInState = 0;

            return false;
        }
    }

    uint8_t result = pm_chargerMenu.Show();

    // Exit
    if (result == 1)
        return true;

    // Edit charge params
    if (result == 2)
    {
        screen::charger_profile::Show(true);
    }

    // Switch profile
    else if (result >= 3)
    {
        result -= 3;
        g_settings.m_chargerProfileNumber = result;
        g_profile.LoadFromEeprom(result);
    }

    DrawBackground();
    Init();
    return false;
}

static const display::UiScreen pm_chargerScreen PROGMEM =
{
    UI_ELEMENT_COUNT,
    &DrawBackground,
    &DrawElements,
    &OnClick,
    &OnChangeValue,
    &OnLongClick
};

void Show()
{
    Init();

    pm_chargerScreen.Show();
}




} // namespace screen::charger

