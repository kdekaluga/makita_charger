#include "../includes.h"

namespace screen::music {

static const char pm_title[] PROGMEM = "Music player";
static const char pm_exit[] PROGMEM = "Exit";

uint8_t DrawItem(uint8_t x, uint8_t y, uint8_t nItem)
{
    if (!nItem)
        return display::PrintString(x, y, pm_exit);

    const char *text = reinterpret_cast<const char*>(pgm_read_word(&sound::pm_melodies[nItem - 1].m_name));
    return display::PrintString(x, y, text);
}

uint8_t GetItemWidth(uint8_t nItem)
{
    if (!nItem)
        return display::GetTextWidth(pm_exit);

    const char *text = reinterpret_cast<const char*>(pgm_read_word(&sound::pm_melodies[nItem - 1].m_name));
    return display::GetTextWidth(text);
}

static const display::Menu pm_playerMenu PROGMEM =
{
    nullptr,
    &DrawItem,
    &GetItemWidth,
    MELODIES_COUNT + 1,
    pm_title
};

void Show()
{
    uint8_t nItem = 0;
    for (;;)
    {
        nItem = pm_playerMenu.Show(nItem);
        if (!nItem)
            break;

        sound::PlayMusic(nItem - 1);
    }

    sound::StopMusic();
}

} // namespace screen::music
