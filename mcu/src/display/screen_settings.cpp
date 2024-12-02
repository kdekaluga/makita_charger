#include "../includes.h"

namespace screen::settings {

int8_t DrawBackgroundPage0();
int8_t DrawBackgroundPage1();
int8_t DrawBackgroundPage2();
int8_t DrawBackgroundPage3();

enum EUiElements: int8_t
{
    // Page 1
    UI_BEEP_LENGTH = 0,
    UI_BEEP_VOLUME,
    UI_SOUND_VOLUME,
    UI_START_SOUND,
    UI_FINISH_SOUND,

    // Page 2
    UI_INTERRUPT_SOUND,
    UI_BAD_BATTERY_SOUND,

    // Page 3
    UI_FAN_PWM_MIN,
    UI_FAN_PWM_MAX,
    UI_FAN_TEMP_STOP,
    UI_FAN_TEMP_START,
    UI_FAN_TEMP_MAX,

    // Page 4
    UI_FAN_POWER_STOP,
    UI_FAN_POWER_START,
    UI_FAN_POWER_MAX,

    UI_ELEMENT_COUNT,
};

constexpr uint8_t YHeader = 23;
constexpr uint8_t YLine1 = 56;
constexpr uint8_t YLine2 = 84;
constexpr uint8_t YLine3 = 112;
constexpr uint8_t YLine4 = 140;
constexpr uint8_t YLine5 = 168;
constexpr uint8_t YLine6 = 196;
constexpr uint8_t YLine7 = 224;

// ***

struct SPageInfo
{
    int8_t m_uiElementsCount;
    display::UiScreen::UiDrawBackgroundFunc m_drawBackGroundFunc;
};

struct SValue
{
    enum EType: uint8_t
    {
        U8_2D,
        U8_3D,
        U8_PWR,
        SOUND,
    };

    EType m_type;
    uint8_t m_y;
    int8_t m_uiPosition;
    void* m_address;
    uint8_t m_min;
    uint8_t m_max;

    static constexpr uint8_t X2Digits = 240 - 7 - 13*2;
    static constexpr uint8_t X3Digits = 240 - 7 - 13*3;
    static constexpr uint8_t XPower = 240 - 7 - 13*3 - 22;

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

        if (type == U8_PWR)
        {
            // Multiply by 1.18 = value + value*0.18
            uint16_t value16 = static_cast<uint16_t>(value)*151 >> 7;
            utils::I16ToString(value16, g_buffer, 4);
            g_buffer[5] = 'W';
            display::PrintStringRam(XPower, y, g_buffer + 2, 4);
            return;
        }

        utils::I8ToStringSpaces(value);
        if (type == U8_2D)
            display::PrintStringRam(X2Digits, y, g_buffer + 1, 2);
        else
            display::PrintStringRam(X3Digits, y, g_buffer, 3);
    }

    void Change(int8_t delta) const
    {
        EType type = static_cast<EType>(pgm_read_byte(&m_type));
        uint8_t* pValue = reinterpret_cast<uint8_t*>(pgm_read_word(&m_address));

        if (type == SOUND)
        {
            uint8_t newValue = utils::ChangeI8ByDelta(*pValue, delta, 0, MELODIES_COUNT - 1);
            sound::PlayMusic(newValue);
            *pValue = newValue;

            return;
        }
        
        *pValue = utils::ChangeI8ByDelta(*pValue, delta, pgm_read_byte(&m_min), pgm_read_byte(&m_max));
    }
};

// ***

static const SValue pm_values[] PROGMEM =
{
    // Page 1, 5 values
    {SValue::U8_2D, YLine1, UI_BEEP_LENGTH, &g_settings.m_keyBeepLength, 0, 30},
    {SValue::U8_2D, YLine2, UI_BEEP_VOLUME, &g_settings.m_keyBeepVolume, 0, 15},
    {SValue::U8_2D, YLine3, UI_SOUND_VOLUME, &g_settings.m_musicVolume, 0, 15},
    {SValue::SOUND, YLine5, UI_START_SOUND, &g_settings.m_chargeStartMusic},
    {SValue::SOUND, YLine7, UI_FINISH_SOUND, &g_settings.m_chargeEndMusic},

    // Page 2, 2 values
    {SValue::SOUND, YLine2, UI_INTERRUPT_SOUND, &g_settings.m_chargeInterruptedMusic},
    {SValue::SOUND, YLine4, UI_BAD_BATTERY_SOUND, &g_settings.m_badBatteryMusic},

    // Page 3, 5 values
    {SValue::U8_2D, YLine2, UI_FAN_PWM_MIN, &g_settings.m_fanPwmMin, 3, 64},
    {SValue::U8_3D, YLine3, UI_FAN_PWM_MAX, &g_settings.m_fanPwmMax, 10, 128},
    {SValue::U8_2D, YLine5, UI_FAN_TEMP_STOP, &g_settings.m_fanTempStop, 25, 60},
    {SValue::U8_2D, YLine6, UI_FAN_TEMP_START, &g_settings.m_fanTempMin, 30, 60},
    {SValue::U8_2D, YLine7, UI_FAN_TEMP_MAX, &g_settings.m_fanTempMax, 45, 90},

    // Page 4, 3 values
    {SValue::U8_PWR, YLine2, UI_FAN_POWER_STOP, &g_settings.m_fanPowStop, 0, 255},
    {SValue::U8_PWR, YLine3, UI_FAN_POWER_START, &g_settings.m_fanPowMin, 0, 255},
    {SValue::U8_PWR, YLine4, UI_FAN_POWER_MAX, &g_settings.m_fanPowMax, 0, 255},
};

static const SPageInfo pm_pages[] PROGMEM =
{
    {5, &DrawBackgroundPage0},
    {2, &DrawBackgroundPage1},
    {5, &DrawBackgroundPage2},
    {3, &DrawBackgroundPage3},
};

// ***

int8_t DrawBackgroundPage0()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(77, YHeader, S, "Settings", 8),
        DRO_STR(240 - 7 - 13*2 - 7, YHeader, S, "1/4", 3),

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

int8_t DrawBackgroundPage1()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_STR(240 - 7 - 13*2 - 7, YHeader, S, "2/4", 3),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 30, 240, 210,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, YLine1, S, "Charge break sound:", 19),
        DRO_STR(7, YLine3, S, "Bad battery sound:", 18),
        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);
    return 0;
}

int8_t DrawBackgroundPage2()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_STR(240 - 7 - 13*2 - 7, YHeader, S, "3/4", 3),

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
    return 0;
}

int8_t DrawBackgroundPage3()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_STR(240 - 7 - 13*2 - 7, YHeader, S, "4/4", 3),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 30, 240, 210,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, YLine1, S, "Fan power settings", 18),
        DRO_STR(28, YLine2, S, "stop:", 5),
        DRO_STR(28, YLine3, S, "start:", 6),
        DRO_STR(28, YLine4, S, "maximum:", 8),
        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);
    return 0;
}

void DrawElements(int8_t cursorPosition, uint8_t ticksElapsed)
{
    display::SetSans12();

    int8_t pageStart = 0;
    int8_t pureCursorPosition = cursorPosition & 0x3F;
    for(const SPageInfo* pageInfo = pm_pages;; ++pageInfo)
    {
        int8_t count = pgm_read_byte(&pageInfo->m_uiElementsCount);
        int8_t nextPageStart = pageStart + count;
        if (pureCursorPosition >= nextPageStart)
        {
            pageStart = nextPageStart;
            continue;
        }

        if (pureCursorPosition != g_previousCursorPosition)
        {
            sound::StopMusic();

            // Did cursor move to a new page?
            if (g_previousCursorPosition < pageStart || g_previousCursorPosition >= nextPageStart)
            {
                const auto drawBackground = reinterpret_cast<display::UiScreen::UiDrawBackgroundFunc>
                    (pgm_read_word(&pageInfo->m_drawBackGroundFunc));
                drawBackground();
            }

            g_previousCursorPosition = pureCursorPosition;
        }

        // Draw page elements
        while(pageStart < nextPageStart)
            pm_values[pageStart++].Draw(cursorPosition);

        break;
    }
}

bool OnClick(int8_t cursorPosition)
{
    return true;
}

void OnChangeValue(int8_t cursorPosition, int8_t delta)
{
    pm_values[cursorPosition & 0x3F].Change(delta);
}

bool OnLongClick(int8_t cursorPosition)
{
    return true;
}

static const display::UiScreen pm_settingsScreen PROGMEM =
{
    UI_ELEMENT_COUNT,
    &DrawBackgroundPage0,
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
