#include "../includes.h"

namespace screen::settings {

#define UI_ELEMENT_COUNT 7

#define UI_BEEP_LENGTH 0
#define UI_BEEP_VOLUME 1
#define UI_SOUND_VOLUME 2
#define UI_START_SOUND 3
#define UI_FINISH_SOUND 4
#define UI_INTERRUPT_SOUND 5
#define UI_BAD_BATTERY_SOUND 6

int8_t DrawBackground()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_FILLRECT | 1, 0, 0, 240, 30,
        DRO_STR(77, 23, S, "Settings", 8),
        DRO_STR(240 - 7 - 13*2 - 7, 23, S, "1/2", 3),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 30, 240, 210,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, 56, S, "Key beep length", 15),
        DRO_STR(7, 84, S, "Key beep volume", 15),
        DRO_STR(7, 112, S, "Sound volume", 12),
        DRO_STR(7, 140, S, "Charge start sound", 18),
        DRO_STR(7, 196, S, "Charge finish sound", 19),

        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);

    return 0;
}

void DrawBackgroundPage1()
{
    static const uint8_t pm_bgObjects[] PROGMEM =
    {
        DRO_STR(240 - 7 - 13*2 - 7, 23, S, "2/2", 3),

        DRO_BGCOLOR(CLR_BLACK),
        DRO_FILLRECT | 1, 0, 30, 240, 210,

        DRO_FGCOLOR(CLR_GRAY),
        DRO_STR(7, 56, S, "Charge stop sound", 17),
        DRO_STR(7, 112, S, "Bad battery sound", 17),
        DRO_END
    };
    display::DrawObjects(pm_bgObjects, CLR_RED_BEAUTIFUL, CLR_WHITE);
}

uint8_t GetPageNumber(int8_t cursorPosition)
{
    if (cursorPosition <= UI_FINISH_SOUND)
        return 0;

    return 1;
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
                DrawBackground();
            else
                DrawBackgroundPage1();
        }

        g_previousCursorPosition = pureCursorPosition;
    }

    const auto DrawSoundName = [&](uint8_t uiElement, uint8_t y, uint8_t sound)
    {
        display::SetUiElementColors(cursorPosition, uiElement);
        const char* text = reinterpret_cast<const char*>(pgm_read_word(&sound::pm_melodies[sound].m_name));
        uint8_t width = display::GetTextWidth(text);
        display::FillRect(7, y - 21, 240 - 14 - width, 27, display::g_bgColor);
        display::PrintString(240 - 7 - width, y, text);
    };

    if (!nPage)
    {
        display::SetUiElementColors(cursorPosition, UI_BEEP_LENGTH);
        utils::I8ToStringSpaces(g_settings.m_keyBeepLength);
        display::PrintStringRam(240 - 7 - 13*2, 56, g_buffer + 1, 2);

        display::SetUiElementColors(cursorPosition, UI_BEEP_VOLUME);
        utils::I8ToStringSpaces(g_settings.m_keyBeepVolume);
        display::PrintStringRam(240 - 7 - 13*2, 84, g_buffer + 1, 2);

        display::SetUiElementColors(cursorPosition, UI_SOUND_VOLUME);
        utils::I8ToStringSpaces(g_settings.m_musicVolume);
        display::PrintStringRam(240 - 7 - 13*2, 112, g_buffer + 1, 2);

        DrawSoundName(UI_START_SOUND, 168, g_settings.m_chargeStartMusic);
        DrawSoundName(UI_FINISH_SOUND, 224, g_settings.m_chargeEndMusic);
    }
    else
    {
        DrawSoundName(UI_INTERRUPT_SOUND, 84, g_settings.m_chargeInterruptedMusic);
        DrawSoundName(UI_BAD_BATTERY_SOUND, 140, g_settings.m_badBatteryMusic);
    }
}

bool OnClick(int8_t cursorPosition)
{
    return true;
}

void OnChangeValue(int8_t cursorPosition, int8_t delta)
{
    const auto ChangeSound = [&](uint8_t& sound)
    {
        sound = utils::ChangeI8ByDelta(sound, delta, 0, MELODIES_COUNT - 1);
        sound::PlayMusic(sound);
    };

    switch ((cursorPosition))
    {
    case UI_BEEP_LENGTH:
        g_settings.m_keyBeepLength = utils::ChangeI8ByDelta(g_settings.m_keyBeepLength, delta, 0, 30);
        break;
    
    case UI_BEEP_VOLUME:
        g_settings.m_keyBeepVolume = utils::ChangeI8ByDelta(g_settings.m_keyBeepVolume, delta, 0, 15);
        break;

    case UI_SOUND_VOLUME:
        g_settings.m_musicVolume = utils::ChangeI8ByDelta(g_settings.m_musicVolume, delta, 0, 15);
        break;

    case UI_START_SOUND:
        ChangeSound(g_settings.m_chargeStartMusic);
        break;

    case UI_FINISH_SOUND:
        ChangeSound(g_settings.m_chargeEndMusic);
        break;

    case UI_INTERRUPT_SOUND:
        ChangeSound(g_settings.m_chargeInterruptedMusic);
        break;

    case UI_BAD_BATTERY_SOUND:
        ChangeSound(g_settings.m_badBatteryMusic);
        break;
    }
}

bool OnLongClick(int8_t cursorPosition)
{
    return true;
}

static const display::UiScreen pm_settingsScreen PROGMEM =
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
    g_previousCursorPosition = 0;
    pm_settingsScreen.Show();
}


} // namespace screen::settings
