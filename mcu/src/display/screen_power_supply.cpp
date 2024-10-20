#include "../includes.h"

namespace screen::psupply {

#define UI_OUT 0
#define UI_TIME_CAPACITY 1
#define UI_VOLTAGE1 2
#define UI_VOLTAGE2 3
#define UI_VOLTAGE3 4
#define UI_VOLTAGE4 5
#define UI_CURRENT1 6
#define UI_CURRENT2 7
#define UI_CURRENT3 8
#define UI_ELEMENT_COUNT 9

int8_t DrawBackground()
{
    static const uint8_t pm_bgObject[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(26, 6 + 17, S, "POWER SUPPLY", 12),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 2, 32, 236, 178,

        DRO_BGCOLOR(CLR_BLUE),
        DRO_FILLRECT | 6,
            0, 30, 2, 180,
            238, 30, 2, 180,
            2, 30, 236, 2,
            136, 32, 2, 120,
            2, 152, 236, 2,
            0, 210, 240, 30,
        
        DRO_STR(18, 214 + 19, S, "Set:", 4),
        DRO_STR(71 + 13*4 + 6, 214 + 19, S, "V", 1),
        DRO_STR(156 + 13*3 + 6, 214 + 19, S, "A", 1),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FGCOLOR(CLR_VOLTAGE),
        DRO_STR(11 + 19*4 + 9, 36 + 5 + 25, L, "V", 1),
        DRO_FGCOLOR(CLR_CURRENT),
        DRO_STR(11 + 19*4 + 9, 74 + 5 + 25, L, "A", 1),
        DRO_FGCOLOR(CLR_WATTAGE),
        DRO_STR(11 + 19*4 + 9, 112 + 5 + 25, L, "W", 1),

        DRO_END
    };
    display::DrawObjects(pm_bgObject, CLR_RED_BEAUTIFUL, CLR_WHITE);

    return 0;
}

void DrawMode(int8_t cursorPosition)
{
    const auto DrawObjects = [](const uint8_t* objects, bool active, bool cursor)
    {
        uint16_t bgColor, fgColor;
        if (active)
        {
            bgColor = CLR_BG_ACTIVE;
            fgColor = CLR_WHITE;
        }
        else
        {
            bgColor = CLR_BG_INACTIVE;
            fgColor = CLR_BLACK;
        }

        if (cursor)
            bgColor = CLR_BG_CURSOR;

        display::DrawObjects(objects, bgColor, fgColor);
    };

    uint8_t pidMode = g_pidMode;

    // CV
    // Rect: (142, 36, 92, 36)
    // Text width = 17(C) + 15(V) = 32, height = 18
    static const uint8_t pm_cvObjects[] PROGMEM =
    {
        DRO_STR(172, 36 + 9 + 17, S, "CV", 2),
        DRO_FILLRECT | 4,
            172, 36, 32, 5,
            172, 36 + 5 + 27, 32, 4,
            142, 36, 30, 36,
            204, 36, 30, 36,
        DRO_END
    };
    DrawObjects(pm_cvObjects, pidMode == PID_MODE_CV, false);

    // CC
    // Rect: (142, 74, 92, 36)
    // Text width = 17(C) + 17(V) = 34, height = 18
    static const uint8_t pm_ccObjects[] PROGMEM =
    {
        DRO_STR(171, 74 + 9 + 17, S, "CC", 2),
        DRO_FILLRECT | 4,
            171, 74, 34, 5,
            171, 74 + 5 + 27, 34, 4,
            142, 74, 29, 36,
            205, 74, 29, 36,
        DRO_END
    };
    DrawObjects(pm_ccObjects, pidMode == PID_MODE_CC, false);

    // OUT
    // Rect: (142, 112, 92, 36)
    // Text width = 19(O) + 17(U) + 15(T) = 51, height = 18
    static const uint8_t pm_outObjects[] PROGMEM =
    {
        DRO_STR(163, 112 + 9 + 17, S, "OUT", 3),
        DRO_FILLRECT | 4,
            163, 112, 51, 5,
            163, 112 + 5 + 27, 51, 4,
            142, 112, 21, 36,
            214, 112, 20, 36,
        DRO_END
    };
    DrawObjects(pm_outObjects, g_outOn, cursorPosition == UI_OUT);
}

void DrawMeasurements(int8_t cursorPosition)
{
    cli();
    uint16_t voltage = g_adcVoltageAverage;
    uint16_t current = g_adcCurrentAverage;
    int16_t tempBoard = g_temperatureBoard;
    sei();

    display::SetSans18();

    // Voltage
    display::SetColors(CLR_BLACK, CLR_VOLTAGE);
    voltage = g_settings.AdcVoltageToDisplayX1000(voltage);
    utils::VoltageToString(voltage, true);
    display::PrintStringRam(11, 36 + 5 + 25, g_buffer, 5);

    // Current
    display::SetColor(CLR_CURRENT);
    current = g_settings.AdcCurrentToDisplayX1000(current);
    utils::CurrentToString(current);
    display::PrintStringRam(11 + 19, 74 + 5 + 25, g_buffer, 4);

    // Wattage
    display::SetColor(CLR_WATTAGE);
    utils::WattageToString(voltage, current);
    display::PrintStringRam(11, 112 + 5 + 25, g_buffer, 5);

    // Capacity
    constexpr uint8_t yCapacity = 158 + 19;
    display::SetSans12();
    display::SetColors(cursorPosition == UI_TIME_CAPACITY ? CLR_BG_CURSOR : CLR_BLACK, CLR_WHITE);
    utils::CapacityToString();
    uint8_t x = display::PrintStringRam(8, yCapacity, g_buffer, 8);
    display::FillRect(x, yCapacity - 21, 141 - x, 27, display::g_bgColor);

    // Time
    utils::TimeToString();
    display::PrintStringRam(141, yCapacity, g_buffer, 8);

    // Temperature
    display::SetColors(CLR_BLACK, utils::GetBoardTempColor(tempBoard));
    utils::TemperatureToString(utils::TemperatureToDisplayX100(tempBoard));
    g_buffer[0] = 'B';
    display::PrintStringRam(8, yCapacity + 27, g_buffer, 6);
}

void DrawSettables(int8_t cursorPosition)
{
    uint8_t nSelected = cursorPosition - UI_VOLTAGE1;
    utils::VoltageToString(g_settings.m_psVoltageX1000, nSelected != 0);
    display::DrawSettableDecimal(71, 214 + 19, 5, nSelected, CLR_WHITE, CLR_BLUE);

    utils::CurrentToString(g_settings.m_psCurrentX1000);
    nSelected = cursorPosition - UI_CURRENT1;
    display::DrawSettableDecimal(156, 214 + 19, 4, nSelected, CLR_WHITE, CLR_BLUE);
}

void UpdateTargetValues()
{
    g_pidTargetVoltage = g_settings.DisplayX1000VoltageToAdc(g_settings.m_psVoltageX1000);
    g_pidTargetCurrent = g_settings.DisplayX1000CurrentToAdc(g_settings.m_psCurrentX1000);
}

void DrawElements(int8_t cursorPosition, uint8_t ticksElapsed)
{
    DrawMode(cursorPosition);
    DrawMeasurements(cursorPosition);
    DrawSettables(cursorPosition);
}

bool OnClick(int8_t cursorPosition)
{
    if (cursorPosition == UI_OUT)
    {
        g_outOn = !g_outOn;
        return false;
    }

    if (cursorPosition == UI_TIME_CAPACITY)
    {
        static const char pm_timeResetTitle[] PROGMEM = "Time reset";
        static const char pm_timeResetText[] PROGMEM = "Are you sure\nwant to reset\ntime and capacity\nvalues?";

        display::SetSans12();
        if (display::MessageBox(pm_timeResetTitle, pm_timeResetText, MB_YESNO | MB_DEFAULT_NO) == 0)
            utils::TimeCapacityReset();

        DrawBackground();
        return false;
    }

    return true;
}

void OnChangeValue(int8_t cursorPosition, int8_t delta)
{
    if (cursorPosition >= UI_VOLTAGE1 && cursorPosition <= UI_VOLTAGE4)
    {
        g_settings.m_psVoltageX1000 =
            utils::ChangeI16ByDigit(g_settings.m_psVoltageX1000, UI_VOLTAGE4 + 1 - cursorPosition, delta, 0, MAX_VOLTAGE);
    }
    else if (cursorPosition >= UI_CURRENT1 && cursorPosition <= UI_CURRENT3)
    {
        g_settings.m_psCurrentX1000 =
            utils::ChangeI16ByDigit(g_settings.m_psCurrentX1000, UI_CURRENT3 + 1 - cursorPosition, delta, 0, MAX_CURRENT);
    }

    UpdateTargetValues();
}

bool OnLongClick(int8_t cursorPosition)
{
    return true;
}

static const display::UiScreen pm_powerSupplyScreen PROGMEM =
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
    UpdateTargetValues();

    display::ShowUiScreen(pm_powerSupplyScreen);
}

} // namespace screen::psupply