#include "../data.h"

namespace sound {

struct SMelody
{
    const char* m_name;
    const uint8_t* m_data;
};

#define MELODIES_COUNT 15
extern const SMelody pm_melodies[MELODIES_COUNT] PROGMEM;

extern "C" {
// How many 100 Hz ticks of key beep remains (must be accessed from assembler code)
var uint8_t g_keyBeepLengthLeft;
}

var uint16_t g_currentNoteDivisor;
var uint8_t g_currentNoteDecayPos;
var uint8_t g_noteTicksLeft;
var const uint8_t* g_currentMusicPos;
var uint8_t g_musicSpeed;

void inline Beep()
{
    g_keyBeepLengthLeft = g_settings.m_keyBeepLength;
}

void OnTimer();
void PlayMusic(uint8_t nMelody);
void StopMusic();


} // namespace sound
