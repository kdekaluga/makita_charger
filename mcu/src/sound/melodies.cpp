#include "../includes.h"

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
#define N6(n) NOTE(n, 6)

#define NOTE_END 0
#define NCONT(l) (1 | ((l - 1) << 2))
#define NMUTE(l) (2 | ((l - 1) << 2))

namespace sound {

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
    N2(D6), N2(C6), N2(B5), N2(G5), N2(G5), N1(B5), N1(C6), N1(D6), N1(E6), N1(D6), N1(E6), N2(C6), //56
    N2(B5), N2(C6), N2(A5), N2(A5), NMUTE(2), N2(E6), N2(E6), N2(D6), N2(C6), N2(B5), N2(G5), // 6C
    N2(G5), N1(B5), N1(C6), N1(D6), N1(B5), N2(D6), N2(C6), N2(B5), N2(C6), N2(A5), N2(A5),
    NOTE_END
};

static const SMelodies pm_melodies PROGMEM = 
{
    2,
    {
        {pm_mDassinTitle, pm_mDassin},
        {pm_mDollyTitle, pm_mDolly},
    }
};


} // namespace sound
