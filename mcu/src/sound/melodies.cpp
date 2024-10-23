#include "../includes.h"

// Melody data starts with a melody speed, then all the melody notes go.
// Melody speed is an 8 bit number representing 100 Hz tick count between
// the adjacent notes. It's value can be calculated as:
//
// speed = 100*60/(4*<BPM>)
//
// And is 12 for 125 BMP, 15 for 100 BM, etc.
// 
// Each note is encoded as a single byte where 5 LSBs represent the note pitch
// and remaining 3 MSBs represent the note length. Note pitch 0 corresponds to
// E4 and note pitch 31 corresponds to B6.
//
// Note length defines how many cycles this note will be played, each cycle consists
// of <speed> 100 Hz ticks. The minimum note length is 1 cycle and the maximum length is
// 7 cycles correspondingly. If you need a note to be played longer, see below for the
// note continuation command.
//
// If note length is zero, the note pitch part of the byte (5 LSBs) is treated as command.
// In this case bits 1:0 represent the command code and bits 4:2 represent the command length.
//
// Command 0 stops the playback. It's the last command in a melody.
// Command 1 continues playing of the current note for the (command length + 1) cycles.
//   Thus it's possible to prolong a note for 8 cycles maximum. If you still need a longer
//   note then you need to put more continuation commands in the song (unlikely you'll need it).
// Command 2 mutes the current note for the next (command length + 1) cycles.
//
// That's probably all. Here are some useful macros which simplify melody encoding.

#define NOTE_E4 0
#define NOTE_F4 1
#define NOTE_Fs4 2
#define NOTE_G4 3
#define NOTE_Gs4 4
#define NOTE_A4 5
#define NOTE_As4 6
#define NOTE_B4 7

#define NOTE_C5 8
#define NOTE_Cs5 9
#define NOTE_D5 10
#define NOTE_Ds5 11
#define NOTE_E5 12
#define NOTE_F5 13
#define NOTE_Fs5 14
#define NOTE_G5 15
#define NOTE_Gs5 16
#define NOTE_A5 17
#define NOTE_As5 18
#define NOTE_B5 19

#define NOTE_C6 20
#define NOTE_Cs6 21
#define NOTE_D6 22
#define NOTE_Ds6 23
#define NOTE_E6 24
#define NOTE_F6 25
#define NOTE_Fs6 26
#define NOTE_G6 27
#define NOTE_Gs6 28
#define NOTE_A6 29
#define NOTE_As6 30
#define NOTE_B6 31

#define NOTE(n, l) (NOTE_##n | (l << 5))
#define N(n, l) NOTE(n, l)
#define N1(n) NOTE(n, 1)
#define N2(n) NOTE(n, 2)
#define N3(n) NOTE(n, 3)
#define N4(n) NOTE(n, 4)
#define N5(n) NOTE(n, 5)
#define N6(n) NOTE(n, 6)

#define NOTE_END 0
#define NCONT(l) (1 | ((l - 1) << 2))
#define NMUTE(l) (2 | ((l - 1) << 2))

namespace sound {

static const char pm_mSilenceTitle[] PROGMEM = "Silence";
static const uint8_t pm_mSilence[] PROGMEM =
{
    12,
    NOTE_END
};

static const char pm_mShortBeepTitle[] PROGMEM = "Short beep";
static const uint8_t pm_mShortBeep[] PROGMEM =
{
    15,
    N2(A6),
    NOTE_END
};

static const char pm_mLongBeepTitle[] PROGMEM = "Long beep";
static const uint8_t pm_mLongBeep[] PROGMEM =
{
    15,
    N6(A6),
    NOTE_END
};

static const char pm_mTripleBeepTitle[] PROGMEM = "Triple beep";
static const uint8_t pm_mTripleBeep[] PROGMEM =
{
    6,
    N2(A6), NMUTE(2), N2(A6), NMUTE(2), N2(A6), NMUTE(2),
    NOTE_END
};

static const char pm_mSosTitle[] PROGMEM = "S.O.S.";
static const uint8_t pm_mSos[] PROGMEM =
{
    12,
    N2(A6), N2(A6), N2(A6), NMUTE(2), N4(A6), N4(A6), N4(A6), NMUTE(1), N2(A6), N2(A6), N2(A6),
    NOTE_END
};

static const char pm_mSound1Title[] PROGMEM = "Sound 1";
static const uint8_t pm_mSound1[] PROGMEM =
{
    12,
    N3(A4), N4(E4),
    NOTE_END
};

static const char pm_mSound2Title[] PROGMEM = "Sound 2";
static const uint8_t pm_mSound2[] PROGMEM =
{
    12,
    N2(A4), N2(A4), N2(A4), N4(E4),
    NOTE_END
};

static const char pm_mSound3Title[] PROGMEM = "Sound 3";
static const uint8_t pm_mSound3[] PROGMEM =
{
    12,
    N4(E4),
    NOTE_END
};

static const char pm_mDassinTitle[] PROGMEM = "Joe Dassin";
static const uint8_t pm_mDassin[] PROGMEM =
{
    12,
    N2(E5), N2(A5), N2(B5), N2(C6), N4(B5), N2(A5), N4(E5), NCONT(4), // 16
    N2(E5), N2(A5), N2(B5), N2(C6), N2(E6), N4(G6), N2(F6), N6(B5), NCONT(4), NMUTE(8), NCONT(4), // 3C
    N2(B5), N2(C6), N4(D6), N2(Cs6), N2(D6), N4(E6), N2(D6), N2(E6), N4(F6), NCONT(4), // 58
    N2(E6), N2(D6), N2(C6), N2(B5), N4(B5), N2(A5), N6(C6), NCONT(4), // 70
    NOTE_END
};

static const char pm_mDollyTitle[] PROGMEM = "Dolly song";
static const uint8_t pm_mDolly[] PROGMEM =
{
    10,
    N2(E5), N2(A5), N3(A5), N1(B5), N1(C6), N1(B5), N2(A5), N2(A5), N1(B5), N1(C6), N2(B5), // 12
    N2(G5), N2(G5), N2(B5), N2(C6), N2(A5), N2(A5), NMUTE(2), N2(E5), N2(A5), N3(A5), //27
    N1(B5), N1(C6), N1(B5), N2(A5), N2(A5), N1(C6), N1(D6), N1(E6), N1(D6), N1(E6), N1(D6), N1(E6), // 35
    N1(D6), N1(C6), N1(B5), N2(C6), N2(A5), N2(A5), N1(C6), N1(D6), N2(E6), N2(E6), // 44
    N2(D6), N2(C6), N2(B5), N2(G5), N2(G5), N1(B5), N1(C6), N1(D6), N1(E6), N1(D6), N1(E6), N2(C6), // 56
    N2(B5), N2(C6), N2(A5), N2(A5), NMUTE(2), N2(E6), N2(E6), N2(D6), N2(C6), N2(B5), N2(G5), // 6C
    N2(G5), N1(B5), N1(C6), N1(D6), N1(B5), N2(D6), N2(C6), N2(B5), N2(C6), N2(A5), N2(A5),
    NOTE_END
};

static const char pm_mPinkEveningTitle[] PROGMEM = "Pink Evening";
static const uint8_t pm_mPinkEvening[] PROGMEM =
{
    12,
    N2(E5), N2(C6), N2(B5), N2(A5), N2(G5), N4(A5), N4(E5), N4(A5), N2(G5), N4(A5), N4(E5), // 0:20
    N2(E5), N2(C6), N2(B5), N2(A5), N2(G5), N4(A5), N4(F5), N4(F5), N2(E5), N6(F5), NCONT(2), // 1:00
    N2(A5), N2(C6), N2(A5), N2(C6), N2(A5), N4(C6), N4(B5), N4(B5), N2(A5), N4(A5), N4(G5), // 1:20
    N2(G5), N2(G5), N2(A5), N2(G5), N2(F5), N4(F5), N4(E5), N4(E5), N2(C6), N6(B5), NCONT(2), // 0:00
    N2(E5), N2(C6), N2(B5), N2(A5), N2(G5), N4(A5), N4(E5), N4(A5), N2(G5), N4(A5), N4(E5), // 0:20
    N2(E5), N2(C6), N2(B5), N2(A5), N2(G5), N4(A5), N4(F5), N4(F5), N2(E5), N6(F5), NCONT(2), // 2:00
    N2(A5), N2(C6), N2(A5), N2(C6), N2(A5), N4(D6), N2(D6), N2(C6), N2(B5), N4(A5), N4(B5), N4(C6), // 2:20
    N2(C6), N2(C6), N2(B5), N2(A5), N2(G5), N6(A5), NCONT(8), NCONT(8), // 3:00
    N2(D6), N2(D6), N2(D6), N2(D6), N2(A5), N6(B5), NMUTE(8), NMUTE(8), // 3:20
    N4(E6), N2(E6), N2(D6), N4(D6), N4(C6), N4(C6), N2(C6), N2(B5), N4(B5), N6(A5), // 4:02
    N2(A5), N2(A5), N2(G5), N6(F5), N6(D6), NCONT(2), N4(C6), N6(B5), // 4:20
    N4(D6), N2(D6), N2(C6), N4(C6), N4(B5), N4(B5), N2(B5), N2(A5), N4(A5), N6(Gs5), // 5:02
    N4(Gs5), N2(A5), N2(F6), N6(E6), NCONT(2), NMUTE(8), NMUTE(6), // 5:20
    N4(E6), N2(F6), N2(E6), N4(E6), N4(D6), N4(D6), N2(C6), N2(B5), N6(D6), NCONT(2), // 6:00
    N4(D6), N2(E6), N2(D6), N4(D6), N4(C6), N4(C6), N2(B5), N2(A5), N6(C6), NCONT(2), // 6:20
    N4(C6), N2(D6), N2(C6), N2(C6), N6(F5), NCONT(4), NMUTE(8), NMUTE(6), // 6:42
    N2(B5), N2(C6), N2(B5), N6(A5), NCONT(2),
    NOTE_END
};

static const char pm_mOzoneTitle[] PROGMEM = "O-Zone";
static const uint8_t pm_mOzone[] PROGMEM =
{
    11,
    N4(C5), N4(B4), N4(A4), N2(B4), N2(C5), N2(D5), N4(C5), N6(A4), NMUTE(4), // 20
    N4(B4), N4(A4), N4(Gs4), N2(A4), N2(B4), N2(C5), N4(B4), N6(A4), NMUTE(4), // 00
    N4(C5), N4(B4), N4(A4), N4(C5), N2(D5), N4(C5), N6(A4), NMUTE(4), // 20
    N4(Gs4), N2(Gs4), N2(Gs4), N2(B4), N1(C5), N3(B4), N6(A4), NMUTE(8), NMUTE(4), // 00
    N4(E5), N4(D5), N4(C5), N4(E5), N2(F5), N4(E5), N6(D5), NMUTE(4), // 20
    N4(D5), N4(C5), N4(B4), N4(D5), N2(E5), N4(D5), N6(C5), NMUTE(4), // 00
    N4(E5), N4(D5), N4(C5), N4(B4), N2(A4), N4(G4), N6(F4), NMUTE(4), // 20
    N4(Gs4), N2(Gs4), N2(Gs4), N2(B4), N1(C5), N3(B4), N6(A4),
    NOTE_END
};

static const char pm_mMelody1Title[] PROGMEM = "Melody 1";
static const uint8_t pm_mMelody1[] PROGMEM =
{
    12,
    N4(E6), N2(A5), N4(E6), N2(A5), N1(E6), N1(D6), N2(C6), N4(B5), N2(G5), // 16
    N4(B5), N2(G5), N1(B5), N1(C6), N2(D6), N4(D6), N2(A5), N4(D6), N4(A5), N1(D6), N1(E6), // 30
    N6(F6), N1(E6), N1(D6), N2(C6), N2(B5), N2(A5), N2(C6),
    NOTE_END
};

static const char pm_mMelody2Title[] PROGMEM = "Melody 2";
static const uint8_t pm_mMelody2[] PROGMEM =
{
    16,
    N2(A4), N2(B4), N4(C5), N4(C5), N4(C5), N2(B4), N2(A4), N2(B4), N2(G4), N6(E4), NMUTE(2), // 20
    N2(A4), N2(B4), N4(C5), N4(C5), N5(E5), NMUTE(1), N2(D5), N6(B4), NMUTE(6), // 40
    N2(B4), N2(C5), N4(D5), N4(D5), N4(D5), N2(E5), N2(D5), N2(F5), N2(E5), N6(C5), NMUTE(2), // 60
    N2(A4), N2(B4), N4(C5), N2(C5), N2(D5), N4(B4), NMUTE(2), N2(C5), N6(A4),
    NOTE_END
};

static const SMelody pm_melodies[MELODIES_COUNT] PROGMEM = 
{
    {pm_mSilenceTitle, pm_mSilence},
    {pm_mShortBeepTitle, pm_mShortBeep},
    {pm_mLongBeepTitle, pm_mLongBeep},
    {pm_mTripleBeepTitle, pm_mTripleBeep},
    {pm_mSosTitle, pm_mSos},
    {pm_mSound1Title, pm_mSound1},
    {pm_mSound2Title, pm_mSound2},
    {pm_mSound3Title, pm_mSound3},
    {pm_mDassinTitle, pm_mDassin},
    {pm_mDollyTitle, pm_mDolly},
    {pm_mPinkEveningTitle, pm_mPinkEvening},
    {pm_mOzoneTitle, pm_mOzone},
    {pm_mMelody1Title, pm_mMelody1},
    {pm_mMelody2Title, pm_mMelody2},
};


} // namespace sound
