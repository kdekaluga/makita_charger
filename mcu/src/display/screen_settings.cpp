#include "../includes.h"

namespace screen::settings {

#define UI_ELEMENT_COUNT 12

// Page 1
#define UI_BEEP_LENGTH 0
#define UI_BEEP_VOLUME 1
#define UI_SOUND_VOLUME 2
#define UI_START_SOUND 3
#define UI_FINISH_SOUND 4

// Page 2
#define UI_INTERRUPT_SOUND 5
#define UI_BAD_BATTERY_SOUND 6

// Page 3
#define UI_FAN_PWM_MIN 7
#define UI_FAN_PWM_MAX 8
#define UI_FAN_TEMP_STOP 9
#define UI_FAN_TEMP_START 10
#define UI_FAN_TEMP_MAX 11

constexpr uint8_t YHeader = 23;
constexpr uint8_t YLine1 = 56;
constexpr uint8_t YLine2 = 84;
constexpr uint8_t YLine3 = 112;
constexpr uint8_t YLine4 = 140;
constexpr uint8_t YLine5 = 168;
constexpr uint8_t YLine6 = 196;
constexpr uint8_t YLine7 = 224;

// ***

struct SValue
{
    enum EType: uint8_t
    {
        U8_2D,
        U8_3D,
        SOUND,
    };

    EType m_type;
    uint8_t m_y;
    uint8_t m_uiPosition;
    void* m_address;
    uint8_t m_min;
    uint8_t m_max;

    static constexpr uint8_t X2Digits = 240 - 7 - 13*2;
    static constexpr uint8_t X3Digits = 240 - 7 - 13*3;

    void Draw(int8_t cursorPosition) const
    {
        display::SetUiElementColors(cursorPosition, pgm_read_byte(&m_uiPosition));

        EType type = static_cast<EType>(pgm_read_byte(&m_type));
        uint8_t value = *reinterpret_cast<uint8_t*>(pgm_read_word(&m_address));
        uint8_t y = pgm_read_byte(&m_y);

        if (type == SOUND)
        {
            const char* text = reinterpret_cast<const char*>(pgm_read_word(&sound::pm_melodies[value].m_name));
            uint8_t width = display::GetTextWidth(text);
            display::FillRect(7, y - 21, 240 - 14 - width, 27, display::g_bgColor);
            display::PrintString(240 - 7 - width, y, text);
            return;
        }

        utils::I8ToStringSpaces(value);
        if (type == U8_2D)
            display::PrintStringRam(X2Digits, y, g_buffer + 1, 2);
        else
            display::PrintStringRam(X3Digits, y, g_buffer, 3);
    }

    bool Change(int8_t cursorPosition, int8_t delta) const
    {
        if (pgm_read_byte(&m_uiPosition) != cursorPosition)
            return false;

        EType type = static_cast<EType>(pgm_read_byte(&m_type));
        uint8_t* pValue = reinterpret_cast<uint8_t*>(pgm_read_word(&m_address));

        if (type == SOUND)
        {
            uint8_t newValue = utils::ChangeI8ByDelta(*pValue, delta, 0, MELODIES_COUNT - 1);
            sound::PlayMusic(newValue);
            *pValue = newValue;

            return true;
        }
        
        *pValue = utils::ChangeI8ByDelta(*pValue, delta, pgm_read_byte(&m_min), pgm_read_byte(&m_max));

        return true;
    }
};

// ***

static const SValue pm_values[] PROGMEM =
{
    // Page 1, 5 values [0, 5)
    {SValue::U8_2D, YLine1, UI_BEEP_LENGTH, &g_settings.m_keyBeepLength, 0, 30},
    {SValue::U8_2D, YLine2, UI_BEEP_VOLUME, &g_settings.m_keyBeepVolume, 0, 15},
    {SValue::U8_2D, YLine3, UI_SOUND_VOLUME, &g_settings.m_musicVolume, 0, 15},
    {SValue::SOUND, YLine5, UI_START_SOUND, &g_settings.m_chargeStartMusic},
    {SValue::SOUND, YLine7, UI_FINISH_SOUND, &g_settings.m_chargeEndMusic},

    // Page 2, 2 values [5, 7)
    {SValue::SOUND, YLine2, UI_INTERRUPT_SOUND, &g_settings.m_chargeInterruptedMusic},
    {SValue::SOUND, YLine4, UI_BAD_BATTERY_SOUND, &g_settings.m_badBatteryMusic},

    // Page 3, 5 values [7, 12)
    {SValue::U8_2D, YLine2, UI_FAN_PWM_MIN, &g_settings.m_fanPwmMin, 3, 64},
    {SValue::U8_3D, YLine3, UI_FAN_PWM_MAX, &g_settings.m_fanPwmMax, 10, 128},
    {SValue::U8_2D, YLine5, UI_FAN_TEMP_STOP, &g_settings.m_fanTempStop, 25, 60},
    {SValue::U8_2D, YLine6, UI_FAN_TEMP_START, &g_settings.m_fanTempMin, 30, 60},
    {SValue::U8_2D, YLine7, UI_FAN_TEMP_MAX, &g_settings.m_fanTempMax, 45, 90},
};

// ***

int8_t DrawBackgroundPage1()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(77, YHeader, S, "Settings", 8),
        DRO_STR(240 - 7 - 13*2 - 7, YHeader, S, "1/3", 3),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 30, 240, 210,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, YLine1, S, "Key beep length:", 16),
        DRO_STR(7, YLine2, S, "Key beep volume:", 16),
        DRO_STR(7, YLine3, S, "Sound volume:", 13),
        DRO_STR(7, YLine4, S, "Charge start sound:", 19),
        DRO_STR(7, YLine6, S, "Charge finish sound:", 20),
        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);

    return 0;
}

void DrawBackgroundPage2()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_STR(240 - 7 - 13*2 - 7, YHeader, S, "2/3", 3),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 30, 240, 210,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, YLine1, S, "Charge break sound:", 19),
        DRO_STR(7, YLine3, S, "Bad battery sound:", 18),
        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);
}

void DrawBackgroundPage3()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_STR(240 - 7 - 13*2 - 7, YHeader, S, "3/3", 3),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 30, 240, 210,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, YLine1, S, "Fan PWM settings", 16),
        DRO_STR(28, YLine2, S, "minimum:", 8),
        DRO_STR(28, YLine3, S, "maximum:", 8),
        DRO_STR(7, YLine4, S, "Fan temperatures", 16),
        DRO_STR(28, YLine5, S, "stop:", 5),
        DRO_STR(28, YLine6, S, "start:", 6),
        DRO_STR(28, YLine7, S, "maximum:", 8),
        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);
}

uint8_t GetPageNumber(int8_t cursorPosition)
{
    if (cursorPosition <= UI_FINISH_SOUND)
        return 0;

    if (cursorPosition <= UI_BAD_BATTERY_SOUND)
        return 1;

    return 2;
}

void DrawElements(int8_t cursorPosition, uint8_t ticksElapsed)
{
    display::SetSans12();

    uint8_t pureCursorPosition = static_cast<uint8_t>(cursorPosition & 0x3F);
    uint8_t nPage = GetPageNumber(pureCursorPosition);

    if (pureCursorPosition != g_previousCursorPosition)
    {
        sound::StopMusic();
        if (nPage != GetPageNumber(g_previousCursorPosition))
        {
            if (!nPage)
                DrawBackgroundPage1();
            else if (nPage == 1)
                DrawBackgroundPage2();
            else
                DrawBackgroundPage3();
        }

        g_previousCursorPosition = pureCursorPosition;
    }

    if (!nPage)
    {
        for (uint8_t i = 0; i < 5; ++i)
            pm_values[i].Draw(cursorPosition);
    }
    else if (nPage == 1)
    {
        for (uint8_t i = 5; i < 7; ++i)
            pm_values[i].Draw(cursorPosition);
    }
    else
    {
        for (uint8_t i = 7; i < 12; ++i)
            pm_values[i].Draw(cursorPosition);
    }
}

bool OnClick(int8_t cursorPosition)
{
    return true;
}

void OnChangeValue(int8_t cursorPosition, int8_t delta)
{
    for (const SValue& value: pm_values)
    {
        if (value.Change(cursorPosition, delta))
            return;
    }
}

bool OnLongClick(int8_t cursorPosition)
{
    return true;
}

static const display::UiScreen pm_settingsScreen PROGMEM =
{
    UI_ELEMENT_COUNT,
    &DrawBackgroundPage1,
    &DrawElements,
    &OnClick,
    &OnChangeValue,
    &OnLongClick
};

void Show()
{
    g_previousCursorPosition = 0;
    pm_settingsScreen.Show();
}


} // namespace screen::settings
