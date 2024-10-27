#include "../includes.h"

namespace screen::calibration {

#define UI_ELEMENT_COUNT 17

#define UI_VOLTAGE1 0
#define UI_VOLTAGE4 3
#define UI_VOLTAGE_MULT2 4
#define UI_VOLTAGE_MULT5 7
#define UI_VOLTAGE_OFS 8
#define UI_CURRENT1 9
#define UI_CURRENT3 11
#define UI_CURRENT_MULT2 12
#define UI_CURRENT_MULT5 15
#define UI_CURRENT_OFS 16

void UpdateTargetValues()
{
    g_pidTargetVoltage = g_settings.DisplayX1000VoltageToAdc(g_voltageX1000);
    g_pidTargetCurrent = g_settings.DisplayX1000CurrentToAdc(g_currentX1000);
}

int8_t DrawBackground()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(44, 23, S, "CALIBRATION", 11),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 2,
            2, 58, 236, 56,
            2, 142, 236, 56,

        DRO_BGCOLOR(CLR_BLUE),
        DRO_FILLRECT | 3,
            0, 30, 2, 210,
            238, 30, 2, 210,
            2, 198, 236, 2,

        DRO_BGCOLOR(CLR_DARK_BLUE),
        DRO_FILLRECT | 3,
            2, 30, 236, 28,
            2, 114, 236, 28,
            2, 200, 236, 40,

        DRO_STR(10, 52, S, "Voltage", 7),
        DRO_STR(240 - 10 - 15, 52, S, "V", 1),
        DRO_STR(10, 136, S, "Current", 7),
        DRO_STR(240 - 10 - 16, 136, S, "A", 1),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(24, 80, S, "Multiplier:", 11),
        DRO_STR(24, 107, S, "Offset:", 7),
        DRO_STR(24, 164, S, "Multiplier:", 11),
        DRO_STR(24, 191, S, "Offset:", 7),

        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);

    return 0;
}

void DrawElements(int8_t cursorPosition, uint8_t ticksElapsed)
{
    cli();
    uint16_t voltage = g_adcVoltageAverage;
    uint16_t current = g_adcCurrentAverage;
    sei();

    display::SetSans12();

    uint8_t pidMode = g_pidMode;

    // Set voltage and CV
    uint8_t nSelected = cursorPosition - UI_VOLTAGE1;
    utils::VoltageToString(g_voltageX1000, nSelected != 0);
    display::DrawSettableDecimal(240 - 10 - 15 - 13*4 - 6, 52, 5, nSelected, CLR_WHITE, CLR_DARK_BLUE);
    display::PrintGlyph(display::GetSans12(), 240 - 10 - 15 - 13*4 - 6 - 13, 52,
        pidMode == PID_MODE_CV ? '*' : 127, CLR_WHITE, CLR_DARK_BLUE);

    // Set current
    nSelected = cursorPosition - UI_CURRENT1;
    utils::CurrentToString(g_currentX1000);
    display::DrawSettableDecimal(240 - 10 - 16 - 13*3 - 6, 136, 4, nSelected, CLR_WHITE, CLR_DARK_BLUE);
    display::PrintGlyph(display::GetSans12(), 240 - 10 - 16 - 13*3 - 6 - 13, 136,
        pidMode == PID_MODE_CC ? '*' : 127, CLR_WHITE, CLR_DARK_BLUE);

    // Voltage multiplier
    display::SetColors(CLR_BLACK, CLR_GRAY);
    nSelected = cursorPosition - UI_VOLTAGE_MULT2 + 1;
    utils::I16ToString(g_settings.m_voltage4096Value, g_buffer, 0);
    display::DrawSettableDecimal(240 - 10 - 13*5, 80, 5, nSelected & 0x7F ? nSelected : 5, CLR_GRAY, CLR_BLACK);
    
    // Voltage offset
    utils::I8ToString(g_settings.m_voltageOffset, g_buffer);
    nSelected = cursorPosition - UI_VOLTAGE_OFS + 2;
    display::DrawSettableDecimal(240 - 10 - 13*3, 107, 3, (nSelected & 0x7F) == 2 ? nSelected : 3, CLR_GRAY, CLR_BLACK);

    // Current multiplier
    nSelected = cursorPosition - UI_CURRENT_MULT2 + 1;
    utils::I16ToString(g_settings.m_current4096Value, g_buffer, 0);
    display::DrawSettableDecimal(240 - 10 - 13*5, 164, 5, nSelected & 0x7F ? nSelected : 5, CLR_GRAY, CLR_BLACK);

    // Current offset
    utils::I8ToString(g_settings.m_currentOffset, g_buffer);
    nSelected = cursorPosition - UI_CURRENT_OFS + 2;
    display::DrawSettableDecimal(240 - 10 - 13*3, 191, 3, (nSelected & 0x7F) == 2 ? nSelected : 3, CLR_GRAY, CLR_BLACK);

    // Voltage
    // In the calibration screen we need to display voltage and current
    // with the maximum possible resolution
    display::SetSans18();
    display::SetColors(CLR_DARK_BLUE, CLR_VOLTAGE);
    voltage = g_settings.AdcVoltageToDisplayX1000(voltage);
    utils::I16ToString(voltage, g_buffer, 1);
    g_buffer[5] = g_buffer[4];
    g_buffer[4] = g_buffer[3];
    g_buffer[3] = g_buffer[2];
    g_buffer[2] = '.';
    display::PrintStringRam(10, 231, g_buffer, 6);

    // Current
    display::SetColor(CLR_CURRENT);
    current = g_settings.AdcCurrentToDisplayX1000(current);
    utils::I16ToString(current, g_buffer, 1);
    g_buffer[0] = g_buffer[1];
    g_buffer[1] = '.';
    display::PrintStringRam(240 - 10 - 23 - 19*3 - 9, 231, g_buffer, 5);
}

bool OnClick(int8_t cursorPosition)
{
    return true;
}

void OnChangeValue(int8_t cursorPosition, int8_t delta)
{
    if (cursorPosition < 0)
        return;

    if (cursorPosition <= UI_VOLTAGE4)
    {
        g_voltageX1000 = utils::ChangeI16ByDigit(g_voltageX1000, UI_VOLTAGE4 + 1 - cursorPosition, delta, 0, MAX_VOLTAGE);
    }
    else if (cursorPosition <= UI_VOLTAGE_MULT5)
    {
        g_settings.m_voltage4096Value =
            utils::ChangeI16ByDigit(g_settings.m_voltage4096Value, UI_VOLTAGE_MULT5 - cursorPosition, delta, 18000, 30000);
    }
    else if (cursorPosition == UI_VOLTAGE_OFS)
    {
        g_settings.m_voltageOffset =
            static_cast<int8_t>(utils::ChangeI16ByDigit(g_settings.m_voltageOffset + 256, 0, delta, 256 - 128, 256 + 127));
    }
    else if (cursorPosition <= UI_CURRENT3)
    {
        g_currentX1000 = utils::ChangeI16ByDigit(g_currentX1000, UI_CURRENT3 + 1 - cursorPosition, delta, 0, MAX_CURRENT);
    }
    else if (cursorPosition <= UI_CURRENT_MULT5)
    {
        g_settings.m_current4096Value =
            utils::ChangeI16ByDigit(g_settings.m_current4096Value, UI_CURRENT_MULT5 - cursorPosition, delta, 8000, 16000);
    }
    else if (cursorPosition == UI_CURRENT_OFS)
    {
        g_settings.m_currentOffset =
            static_cast<int8_t>(utils::ChangeI16ByDigit(g_settings.m_currentOffset + 256, 0, delta, 256 - 128, 256 + 127));
    }
    
    UpdateTargetValues();
}

bool OnLongClick(int8_t cursorPosition)
{
    static const char pm_menuTitle[] PROGMEM = "Calibration menu";
    static const char pm_menu0[] PROGMEM = "Return";
    static const char pm_menu1[] PROGMEM = "Reset values";
    static const char pm_menu2[] PROGMEM = "Save and exit";
    static const display::Menu pm_menu PROGMEM =
    {
        nullptr, nullptr, nullptr,
        3,
        pm_menuTitle,
        pm_menu0,
        pm_menu1,
        pm_menu2,
    };
    uint8_t item = pm_menu.Show();
    if (item == 2)
        return true;

    if (item == 1)
        g_settings.ReadFromEeprom();

    DrawBackground();
    return false;
}

static const display::UiScreen pm_calibrationScreen PROGMEM =
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
    g_outOn = true;

    pm_calibrationScreen.Show();
}


} // namespace screen::calibration