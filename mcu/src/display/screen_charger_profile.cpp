#include "../includes.h"

namespace screen::charger_profile {

using screen::settings::g_previousCursorPosition;
using ::charger::g_editorProfile;
using ::charger::g_tempProfile;

#define UI_ELEMENT_COUNT (20 + (4 + 3 + 4 + 2) + (4 + 4 + 3) + 3)
#define UI_PROFILE_NAME 0
#define UI_VOLTAGE 20
#define UI_CURRENT 24
#define UI_MIN_VOLTAGE 27
#define UI_FINISH_CURRENT 31
#define UI_RESTART_VOLTAGE 33
#define UI_OPEN_VOLTAGE 37
#define UI_OPEN_CURRENT 41
#define UI_OPT_CCC 44
#define UI_OPT_3RD_PIN 45
#define UI_OPT_RESTART 46

constexpr uint8_t XPage = 240 - 7 - 13*2 - 7;

constexpr uint8_t YHeader = 23;
constexpr uint8_t YProfile = 53;
constexpr uint8_t YLine1 = 85;
constexpr uint8_t YLine2 = 113;
constexpr uint8_t YLine3 = 141;
constexpr uint8_t YLine4 = 169;
constexpr uint8_t YLine5 = 197;
constexpr uint8_t YLine6 = 225;

static const char pm_pmTitle[] PROGMEM = "Select profile";
static const char pm_pmReturn[] PROGMEM = "Return";
static const char pm_pmReset[] PROGMEM = "Reset changes";
static const char pm_pmSaveExit[] PROGMEM = "Save and exit";

uint8_t MenuDrawItem(uint8_t x, uint8_t y, uint8_t nItem)
{
    if (nItem == 0)
        return display::PrintString(x, y, pm_pmReturn);

    if (nItem == 1)
        return display::PrintString(x, y, pm_pmReset);

    if (nItem == 2)
        return display::PrintString(x, y, pm_pmSaveExit);

    g_tempProfile.LoadFromEeprom(nItem - 3);
    return display::PrintStringRam(x, y, g_tempProfile.m_name, g_tempProfile.m_nameLength);
}

uint8_t MenuGetItemWidth(uint8_t nItem)
{
    if (nItem == 0)
        return display::GetTextWidth(pm_pmReturn);
    
    if (nItem == 1)
        return display::GetTextWidth(pm_pmReset);

    if (nItem == 2)
        return display::GetTextWidth(pm_pmSaveExit);

    g_tempProfile.LoadFromEeprom(nItem - 3);
    return display::GetTextWidthRam(g_tempProfile.m_name, g_tempProfile.m_nameLength);    
}

static const display::Menu pm_editProfilesMenu PROGMEM =
{
    nullptr,
    &MenuDrawItem,
    &MenuGetItemWidth,
    EEPROM_PROFILES_COUNT + 3,
    pm_pmTitle
};

struct SBitOption
{
    uint8_t m_y;
    uint8_t m_uiPosition;
    uint8_t m_value;

    void Draw(int8_t cursorPosition) const
    {
        static const char pm_yes[] PROGMEM = "Yes";
        static const char pm_no[] PROGMEM = "No";
        constexpr uint8_t yesWidth = 16 + 13 + 12;
        constexpr uint8_t noWidth = 18 + 13;
        
        uint8_t y = pgm_read_byte(&m_y);
        display::SetUiElementColors(cursorPosition, pgm_read_byte(&m_uiPosition));
        if (g_editorProfile.m_options & pgm_read_byte(&m_value))
        {
            display::PrintString(240 - 7 - yesWidth, y, pm_yes);
        }
        else
        {
            display::FillRect(240 - 7 - yesWidth, y - 21, yesWidth - noWidth, 27, display::g_bgColor);
            display::PrintString(240 - 7 - noWidth, y, pm_no);
        }
    }
};

static const SBitOption pm_options[] PROGMEM =
{
    {YLine1, UI_OPT_CCC, COPT_CCC_MODE},
    {YLine3, UI_OPT_3RD_PIN, COPT_USE_3RD_PIN},
    {YLine5, UI_OPT_RESTART, COPT_RESTART_CHARGE},
};

// ***

int8_t DrawBackgroundP1()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(26, YHeader, S, "Charger profile", 15),
        DRO_STR(XPage, YHeader, S, "1/3", 3),

        DRO_BGCOLOR(CLR_DARK_BLUE),
        DRO_FILLRECT | 1, 0, 30, 240, 30,

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 60, 240, 180,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, YLine1, S, "Voltage", 7),
        DRO_STR(7, YLine2, S, "Current", 7),
        DRO_STR(7, YLine3, S, "Min. battery voltage", 20),
        DRO_STR(7, YLine5, S, "Charge finish current", 21),

        DRO_FGCOLOR(CLR_WHITE),
        DRO_STR(240 - 7 - 15, YLine1, S, "V", 1),
        DRO_STR(240 - 7 - 16, YLine2, S, "A", 1),
        DRO_STR(240 - 7 - 15, YLine4, S, "V", 1),
        DRO_STR(240 - 7 - 21, YLine6, S, "%", 1),

        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);

    return UI_VOLTAGE;
}

void DrawBackgroundP2()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(26, YHeader, S, "Charger profile", 15),
        DRO_STR(XPage, YHeader, S, "2/3", 3),

        DRO_BGCOLOR(CLR_DARK_BLUE),
        DRO_FILLRECT | 1, 0, 30, 240, 30,

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 60, 240, 180,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, YLine1, S, "Charge restart volt.", 20),
        DRO_STR(7, YLine3, S, "No battery voltage", 18),
        DRO_STR(7, YLine5, S, "No battery current", 18),

        DRO_FGCOLOR(CLR_WHITE),
        DRO_STR(240 - 7 - 15, YLine2, S, "V", 1),
        DRO_STR(240 - 7 - 15, YLine4, S, "V", 1),
        DRO_STR(240 - 7 - 19 - 16, YLine6, S, "mA", 2),

        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);
}

void DrawBackgroundP3()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(26, YHeader, S, "Charger profile", 15),
        DRO_STR(XPage, YHeader, S, "3/3", 3),

        DRO_BGCOLOR(CLR_DARK_BLUE),
        DRO_FILLRECT | 1, 0, 30, 240, 30,

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 60, 240, 180,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, YLine1, S, "CCC mode", 8),
        DRO_STR(7, YLine2, S, "Use Makita 3rd pin", 18),
        DRO_STR(7, YLine4, S, "Restart charge after", 20),
        DRO_STR(7, YLine5, S, "completion", 10),

        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);
}

void DrawPageBackground(uint8_t nPage)
{
    if (!nPage)
        DrawBackgroundP1();
    else if (nPage == 1)
        DrawBackgroundP2();
    else
        DrawBackgroundP3();
}

uint8_t GetPageNumber(int8_t cursorPosition)
{
    if (cursorPosition < UI_RESTART_VOLTAGE)
        return 0;

    if (cursorPosition >= UI_OPT_CCC)
        return 2;

    return 1;
}

void DrawElements(int8_t cursorPosition, uint8_t ticksElapsed)
{
    display::SetSans12();

    uint8_t pureCursorPosition = static_cast<uint8_t>(cursorPosition & 0x3F);
    uint8_t nPage = GetPageNumber(pureCursorPosition);
    if (nPage != GetPageNumber(g_previousCursorPosition))
        DrawPageBackground(nPage);

    g_previousCursorPosition = pureCursorPosition;

    // Profile name (draw in any page because of the cursor)
    uint8_t x = 7;
    for (uint8_t i = 0; i < 20; ++i)
    {
        uint16_t bgColor = CLR_DARK_BLUE, fgColor = CLR_WHITE;
        if ((cursorPosition & 0x7F) == i)
        {
            bgColor = CLR_BG_CURSOR;
            if (cursorPosition & DSD_CURSOR_SKIP)
                fgColor = CLR_BG_CURSOR;
        }

        x += display::PrintGlyph(display::GetSans12(), x, YProfile, g_editorProfile.m_name[i], fgColor, bgColor);
    }
    display::FillRect(x, YProfile - 21, 240 - x, 27, CLR_DARK_BLUE);

    if (!nPage)
    {
        utils::VoltageToString(g_editorProfile.m_chargeVoltageX1000, cursorPosition != UI_VOLTAGE);
        display::DrawSettableDecimal(240 - 7 - 13*4 - 15 - 6, YLine1, 5,
            cursorPosition - UI_VOLTAGE, CLR_WHITE, CLR_BLACK);

        utils::CurrentToString(g_editorProfile.m_chargeCurrentX1000);
        display::DrawSettableDecimal(240 - 7 - 13*3 - 16 - 6, YLine2, 4,
            cursorPosition - UI_CURRENT, CLR_WHITE, CLR_BLACK);

        utils::VoltageToString(g_editorProfile.m_minBatteryVoltageX1000, cursorPosition != UI_MIN_VOLTAGE);
        display::DrawSettableDecimal(240 - 7 - 13*4 - 15 - 6, YLine4, 5,
            cursorPosition - UI_MIN_VOLTAGE, CLR_WHITE, CLR_BLACK);

        utils::I8ToString(g_editorProfile.m_stopChargeCurrentPercent, g_buffer);
        g_buffer[0] = (cursorPosition != UI_FINISH_CURRENT && g_buffer[1] == '0') ? 127 : g_buffer[1];
        g_buffer[1] = g_buffer[2];
        display::DrawSettableDecimal(240 - 7 - 13*2 - 21, YLine6, 2,
            cursorPosition - UI_FINISH_CURRENT, CLR_WHITE, CLR_BLACK);
    }
    else if (nPage == 1)
    {
        utils::VoltageToString(g_editorProfile.m_restartChargeVoltageX1000, cursorPosition != UI_RESTART_VOLTAGE);
        display::DrawSettableDecimal(240 - 7 - 13*4 - 15 - 6, YLine2, 5,
            cursorPosition - UI_RESTART_VOLTAGE, CLR_WHITE, CLR_BLACK);

        utils::VoltageToString(g_editorProfile.m_openVoltageX1000, cursorPosition != UI_OPEN_VOLTAGE);
        display::DrawSettableDecimal(240 - 7 - 13*4 - 15 - 6, YLine4, 5,
            cursorPosition - UI_OPEN_VOLTAGE, CLR_WHITE, CLR_BLACK);

        utils::I16ToString(g_editorProfile.m_openCurrentX1000, g_buffer, cursorPosition != UI_OPEN_CURRENT ? 3 : 2);
        g_buffer[0] = g_buffer[2];
        g_buffer[1] = g_buffer[3];
        g_buffer[2] = g_buffer[4];
        display::DrawSettableDecimal(240 - 7 - 13*3 - 19 - 16, YLine6, 3,
            cursorPosition - UI_OPEN_CURRENT, CLR_WHITE, CLR_BLACK);
    }
    else
    {
        for (const SBitOption& option: pm_options)
            option.Draw(cursorPosition);
    }
}

bool OnClick(int8_t cursorPosition)
{
    // Prevent editing name for the current profile (it's useless)
    if (g_bCurrentProfile && cursorPosition < UI_VOLTAGE)
        return false;

    if (cursorPosition == UI_OPT_CCC)
    {
        g_editorProfile.m_options ^= COPT_CCC_MODE;
        return false;
    }

    if (cursorPosition == UI_OPT_3RD_PIN)
    {
        g_editorProfile.m_options ^= COPT_USE_3RD_PIN;
        return false;
    }

    if (cursorPosition == UI_OPT_RESTART)
    {
        g_editorProfile.m_options ^= COPT_RESTART_CHARGE;
        return false;
    }
    
    return true;
}

void OnChangeValue(int8_t cursorPosition, int8_t delta)
{
    if (cursorPosition < UI_VOLTAGE)
    {
        g_editorProfile.m_name[cursorPosition] =
            utils::ChangeI8ByDelta(g_editorProfile.m_name[cursorPosition], delta, 32, 126);
        return;
    }

    if (cursorPosition < UI_VOLTAGE + 4)
    {
        g_editorProfile.m_chargeVoltageX1000 = utils::ChangeI16ByDigit(g_editorProfile.m_chargeVoltageX1000,
            UI_VOLTAGE + 4 - cursorPosition, delta, 1500, 23500);
        return;
    }

    if (cursorPosition < UI_CURRENT + 3)
    {
        g_editorProfile.m_chargeCurrentX1000 = utils::ChangeI16ByDigit(g_editorProfile.m_chargeCurrentX1000,
            UI_CURRENT + 3 - cursorPosition, delta, 100, MAX_CURRENT);
        return;
    }

    if (cursorPosition < UI_MIN_VOLTAGE + 4)
    {
        g_editorProfile.m_minBatteryVoltageX1000 = utils::ChangeI16ByDigit(g_editorProfile.m_minBatteryVoltageX1000,
            UI_MIN_VOLTAGE + 4 - cursorPosition, delta, 900, 23000);
        return;
    }

    if (cursorPosition < UI_FINISH_CURRENT + 2)
    {
        g_editorProfile.m_stopChargeCurrentPercent = LOBYTE(utils::ChangeI16ByDigit(
            g_editorProfile.m_stopChargeCurrentPercent, UI_FINISH_CURRENT + 1 - cursorPosition, delta, 0, 50));
        return;
    }

    if (cursorPosition < UI_RESTART_VOLTAGE + 4)
    {
        g_editorProfile.m_restartChargeVoltageX1000 = utils::ChangeI16ByDigit(g_editorProfile.m_restartChargeVoltageX1000,
            UI_RESTART_VOLTAGE + 4 - cursorPosition, delta, 1000, 23500);
        return;
    }

    if (cursorPosition < UI_OPEN_VOLTAGE + 4)
    {
        g_editorProfile.m_openVoltageX1000 = utils::ChangeI16ByDigit(g_editorProfile.m_openCurrentX1000,
            UI_OPEN_VOLTAGE + 4 - cursorPosition, delta, 1500, 24000);
        return;
    }

    if (cursorPosition < UI_OPEN_CURRENT + 3)
    {
        g_editorProfile.m_openCurrentX1000 = utils::ChangeI16ByDigit(g_editorProfile.m_openCurrentX1000,
            UI_OPEN_CURRENT + 2 - cursorPosition, delta, 10, 900);
        return;
    }
}

bool OnLongClick(int8_t cursorPosition)
{
    // No menu if we're editing the current profile
    if (g_bCurrentProfile)
    {
        ::charger::g_profile = g_editorProfile;
        return true;
    }

    // Show profile selection menu
    uint8_t result = pm_editProfilesMenu.Show();

    // Reset changes
    if (result == 1)
    {
        g_editorProfile.LoadFromEeprom(g_settings.m_chargerProfileNumber);
    }
    else if (result >= 2)
    {
        g_editorProfile.SaveToEeprom(g_settings.m_chargerProfileNumber);
        
        // Exit
        if (result == 2)
            return true;

        // Load profile
        result -= 3;
        g_settings.m_chargerProfileNumber = result;
        g_editorProfile.LoadFromEeprom(result);
    }
    
    DrawPageBackground(GetPageNumber(cursorPosition));
    return false;
}

static const display::UiScreen pm_profileScreen PROGMEM =
{
    UI_ELEMENT_COUNT,
    &DrawBackgroundP1,
    &DrawElements,
    &OnClick,
    &OnChangeValue,
    &OnLongClick
};

void Show(bool bCurrentProfile)
{
    g_previousCursorPosition = 0;
    g_bCurrentProfile = bCurrentProfile;
    if (bCurrentProfile)
        g_editorProfile = ::charger::g_profile;
    else
        g_editorProfile.LoadFromEeprom(g_settings.m_chargerProfileNumber);

    pm_profileScreen.Show();
}

} // namespace screen::charger_profile
