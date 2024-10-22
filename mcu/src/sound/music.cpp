#include "../includes.h"

namespace sound {

static const uint16_t pm_noteDivisors[] PROGMEM =
{
    // E4 - B4
    3034, 2863, 2703, 2551, 2408, 2273, 2145, 2025,
    // C5 - B5
    1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012,
    // C6 - B6
    956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506,
};

// Notes decay table
static const uint8_t pm_noteDecayVolume[] PROGMEM =
{
    0, 128, 128, 128, 127, 126, 124, 122, 119,
    117, 114, 112, 109, 107, 105, 103, 102, 101, 100, 99, 97, 96, 255,
};

/*
// Old notes decay table
static const uint8_t pm_noteDecayVolume[] PROGMEM =
{
    0, 0, 128, 128, 128, 127, 125, 120, 115, 110,
    105, 100, 95, 90, 85, 83, 80, 77, 75, 73, 70, 67, 64, 255,
};
*/

// Volume table [0..15]
static const uint8_t pm_volumeTable[] PROGMEM =
{
    0, 1, 2, 3, 4, 5, 6, 8, 11, 15, 20, 26, 33, 41, 60, 100
};

void ProcessKeyBeep()
{
    if (!g_keyBeepLengthLeft || !g_settings.m_keyBeepVolume)
        return;

    ICR1H = HIBYTE(1000);
    ICR1L = LOBYTE(1000);

    OCR1AH = 0;
    OCR1AL = (--g_keyBeepLengthLeft ? pgm_read_byte(&pm_volumeTable[g_settings.m_keyBeepVolume]) : 0);
}

void MusicPlayer()
{
    if (--g_noteTicksLeft)
        return;

    const uint8_t* musicPos = g_currentMusicPos;
    if (!musicPos)
        return;

    uint8_t note = pgm_read_byte(musicPos++);
    if (note == 0)
    {
        g_currentMusicPos = 0;
        g_currentNoteDivisor = 0;
        return;
    }

    uint8_t length = note >> 5;
    if (length)
    {
        g_currentNoteDivisor = pgm_read_word(&pm_noteDivisors[note & 0x1F]);
        g_currentNoteDecayPos = 0;
    }
    else
    {
        // Add 1 here since we cannot have zero length
        length = ((note >> 2) & 0x07) + 1;

        // Mute
        if ((note & 0x03) == 2)
            g_currentNoteDivisor = 0;
    }

    g_noteTicksLeft = length*g_musicSpeed;
    g_currentMusicPos = musicPos;
}

void PlayNote()
{
    uint8_t volume = pgm_read_byte(&pm_noteDecayVolume[g_currentNoteDecayPos]);
    if (volume == 255)
        volume = pgm_read_byte(&pm_noteDecayVolume[g_currentNoteDecayPos - 1]);
    else
        ++g_currentNoteDecayPos;

    // Don't play note during a beep sound
    if (g_keyBeepLengthLeft)
        return;

    uint16_t noteDivisor = g_currentNoteDivisor;
    if (!noteDivisor || !volume)
    {
        OCR1AH = 0;
        OCR1AL = 0;
        return;
    }

    constexpr uint8_t musicVolume = 15;
    volume = HIBYTE(static_cast<uint16_t>(musicVolume << 1)*volume);
    if (!volume)
        volume = 1;

    volume = pgm_read_byte(&pm_volumeTable[volume]);

    ICR1H = HIBYTE(noteDivisor);
    ICR1L = LOBYTE(noteDivisor);

    OCR1AH = 0;
    OCR1AL = volume;
}

void OnTimer()
{
    ProcessKeyBeep();
    MusicPlayer();
    PlayNote();
}

void PlayMusic(uint8_t nMelody)
{
    uint8_t melodyCount = pgm_read_byte(&pm_melodies.m_count);
    if (nMelody >= melodyCount)
        return;

    const uint8_t* music = reinterpret_cast<const uint8_t*>(pgm_read_word(&pm_melodies.m_melodies[nMelody].m_data));
    uint8_t speed = pgm_read_byte(music++);
    cli();
    g_currentMusicPos = music;
    g_noteTicksLeft = 1;
    g_musicSpeed = speed;
    sei();
}

void StopMusic()
{
    cli();
    g_currentMusicPos = 0;
    g_currentNoteDivisor = 0;
    sei();
}


} // namespace sound

