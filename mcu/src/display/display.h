#pragma once

#include "../data.h"
#include <avr/pgmspace.h>

// Colors
#define RGB(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))
#define CLR_BLACK 0x0000
#define CLR_RED RGB(255, 0, 0)
#define CLR_GREEN RGB(0, 255, 0)
#define CLR_BLUE RGB(0, 0, 0xFF)
#define CLR_DARK_BLUE RGB(0, 0, 128)
#define CLR_WHITE 0xFFFF
#define CLR_GRAY RGB(192, 192, 192)

#define CLR_RED_BEAUTIFUL RGB(237, 20, 91)

#define CLR_VOLTAGE RGB(128, 255, 0)
#define CLR_CURRENT RGB(255, 255, 0)
#define CLR_WATTAGE RGB(255, 128, 64)

#define CLR_BG_ACTIVE RGB(0, 128, 128)
#define CLR_BG_INACTIVE RGB(0, 64, 64)

#define CLR_BG_CURSOR RGB(0xC0, 0xC0, 0x00)
#define CLR_FG_CURSOR CLR_BLACK

namespace display {

// Common text strings
extern const char pm_warning[] PROGMEM;
extern const char pm_failure[] PROGMEM;
extern const char pm_error[] PROGMEM;
extern const char pm_yes[] PROGMEM;
extern const char pm_no[] PROGMEM;

// The font representation idea was taken from the ADAFruit GFX library
// (with little improvements)

// Describes a single font glyph (7 bytes)
struct Glyph
{
    // Glyph offset from the start of the bitmap data.
    uint16_t m_offset;

    // Glyph width in pixels
    uint8_t m_width;

    // Glyph height in pixels
    uint8_t m_height;

    // Distance in pixels to advance cursor for the next char
    uint8_t m_xAdvance;

    // X offset to the UL corner, cannot be negative
    uint8_t m_xOffset;

    // Y offset to the UL corner
    int8_t m_yOffset;
};

// Describes the entire font (6 bytes + bitmap array)
struct Font
{
    // Font glyphs array
    const Glyph *m_glyphs;

    // Last and first char represented in the glyphs array
    // Warning, the last char goes first, that's easier for the printing routine
    uint8_t m_lastChar;
    uint8_t m_firstChar;
  
    // Y offset to the upper line which needs to be drawn for the font
    int8_t m_yFirstLineOffset;

    // Distance in pixels for the next text line
    uint8_t m_yAdvance;

    // Font glyph bitmaps, concatenated. The bitmaps have 1 bit depth and are
    // stored in the left-right-up-down order, MSB first. Each glyph starts from
    // bit 7 of its own byte
    uint8_t m_bitmaps[];
};

// 8-bit rectangle
struct Rect
{
    uint8_t x, y;
    uint8_t width, height;
};

// Initializes the ST7789 driver
void Init();

// Fonts and color functions
uint8_t GetTextWidth(const char* text);
uint8_t GetTextWidthRam(const char* text, uint8_t length);
void SetSans12();
void SetSans18();
const Font* GetSans12();
const Font* GetSans18();
void SetBgColor(uint16_t bgColor);
void SetColor(uint16_t color);
void SetColors(uint16_t bgColor, uint16_t fgColor);
void SetUiElementColors(int8_t cursorPosition, int8_t nElement);
uint8_t PrintGlyph(uint8_t x, uint8_t y, uint8_t code);

// Draws count characters of a decimal string from g_buffer, each digit of which
// can be independently highlighted.
// cursorPos selects which digit is highlighted (0 - first, 1 - second, etc.). The dot
// symbol (".") is skipped when digits are counted.
// If DSD_CURSOR_HIDDEN is specified (in cursorPos), all digits are drawn normally.
// If DSD_CURSOR_SKIP is specified (in cursorPos), highlighted digit is drawn using the
// background cursor color (i.e. is not visible).
uint8_t DrawSettableDecimal(uint8_t x, uint8_t y, uint8_t count, uint8_t cursorPos, uint16_t fgColor, uint16_t bgColor);

#define DSD_CURSOR_SKIP 0x80
#define DSD_CURSOR_HIDDEN 0x40

// Displays a message box with the specified caption and text (both located in the program memory).
// Caption must be a single line string, whereas text can be multiline. The '\n' symbol is used
// as line breaks.
// Message box can contain no, one (OK) or two (YES/NO) buttons. In the first case
// the function returns immediately after drawing the box.
// In the one-button mode the function returns after the users click the encoder.
// If two buttons are displayed, the user can select a button by rotating the encoder
// and confirm their selection by clicking it. In this case MessageBox() returns
// the index of the selected button (0 = YES, 1 = NO).
// In the two-button mode the first (YES) button is selected initially. You can specify the
// MB_DEFAULT_NO flag to initially select the second (NO) one.
uint8_t MessageBox(const char* caption, const char* text, uint8_t flags);

#define MB_OK 0x01
#define MB_YESNO 0x02
#define MB_DEFAULT_NO 0x04
#define MB_INFO 0x00
#define MB_ERROR 0x08
#define MB_WARNING 0x10

// *** UI screen ***

class UiScreen
{
public:
    // Returns initial cursor position
    using UiDrawBackgroundFunc = int8_t(*)();
    using UiDrawElementsFunc = void(*)(int8_t cursorPosition, uint8_t ticksElapsed);
    using UiOnClickElementFunc = bool(*)(int8_t cursorPosition);
    using UiOnChangeElementFunc = void(*)(int8_t cursorPosition, int8_t delta);
    using UiOnLongClickFunc = bool(*)(int8_t cursorPosition);

    int8_t m_elementCount;
    UiDrawBackgroundFunc m_drawBackgroundFunc;
    UiDrawElementsFunc m_drawElementsFunc;
    UiOnClickElementFunc m_onClickElementFunc;
    UiOnChangeElementFunc m_onChangeElementFunc;
    UiOnLongClickFunc m_onLongClickFunc;

    void Show() const;

private:
    int8_t DrawBackground() const;
    void DrawElements(int8_t cursorPosition, uint8_t ticksElapsed) const;
    bool OnClickElement(int8_t cursorPosition) const;
    void OnChangeElement(int8_t cursorPosition, int8_t delta) const;
    bool OnLongClick(int8_t cursorPosition) const;

    static bool CheckFailureState();
};

// *** Draw objects ***

#define DRO_CMDMASK 0xE0
#define DRO_PARAMMASK 0x1F

#define DRO_END 0x00
#define _DRO_BGCOLOR 0x20
#define _DRO_FGCOLOR 0x40
#define DRO_FILLRECT 0x60
#define _DRO_STRING_S 0x80
#define _DRO_STRING_L 0xA0

#define _DRO_STR1(str) str[0]
#define _DRO_STR2(str) _DRO_STR1(str), str[1]
#define _DRO_STR3(str) _DRO_STR2(str), str[2]
#define _DRO_STR4(str) _DRO_STR3(str), str[3]
#define _DRO_STR5(str) _DRO_STR4(str), str[4]
#define _DRO_STR6(str) _DRO_STR5(str), str[5]
#define _DRO_STR7(str) _DRO_STR6(str), str[6]
#define _DRO_STR8(str) _DRO_STR7(str), str[7]
#define _DRO_STR9(str) _DRO_STR8(str), str[8]
#define _DRO_STR10(str) _DRO_STR9(str), str[9]
#define _DRO_STR11(str) _DRO_STR10(str), str[10]
#define _DRO_STR12(str) _DRO_STR11(str), str[11]
#define _DRO_STR13(str) _DRO_STR12(str), str[12]
#define _DRO_STR14(str) _DRO_STR13(str), str[13]
#define _DRO_STR15(str) _DRO_STR14(str), str[14]
#define _DRO_STR16(str) _DRO_STR15(str), str[15]
#define _DRO_STR17(str) _DRO_STR16(str), str[16]
#define _DRO_STR18(str) _DRO_STR17(str), str[17]
#define _DRO_STR19(str) _DRO_STR18(str), str[18]
#define _DRO_STR20(str) _DRO_STR19(str), str[19]
#define _DRO_STR21(str) _DRO_STR20(str), str[20]
#define _DRO_STR22(str) _DRO_STR21(str), str[21]

#define DRO_STR(x, y, size, str, count) _DRO_STRING_##size | count, x, y, _DRO_STR##count(str)
#define DRO_FGCOLOR(color) _DRO_FGCOLOR, static_cast<uint8_t>(LOBYTE(color)), static_cast<uint8_t>(HIBYTE(color))
#define DRO_BGCOLOR(color) _DRO_BGCOLOR, static_cast<uint8_t>(LOBYTE(color)), static_cast<uint8_t>(HIBYTE(color))

void DrawObjects(const uint8_t* objects, uint16_t bgColor, uint16_t fgColor);

// *** Menu ***

class Menu
{
public:
    using FuncGetItemCount = uint8_t(*)();
    using FuncDrawItem = uint8_t(*)(uint8_t x, uint8_t y, uint8_t nItem);
    using FuncGetItemWidth = uint8_t(*)(uint8_t nItem);

    FuncGetItemCount m_getItemCount;
    FuncDrawItem m_drawItem;
    FuncGetItemWidth m_getItemWidth;

    uint8_t m_itemCount;
    const char* m_title;
    const char* m_itemNames[];

    uint8_t Show() const;
    uint8_t Show(uint8_t selectedItem) const;

private:
    uint8_t GetItemCount() const;
    uint8_t DrawItem(uint8_t x, uint8_t y, uint8_t nItem) const;
    uint8_t GetItemWidth(uint8_t nItem) const;
};

// *** Assembler routines ***

extern "C" {

// Sends a command or data to ST7789
// Waits for a previous transfer completion (SPIF in SPSR), so the tansfer must be started prior
// to this function call. Quite slow.
void SendCommand(uint8_t command);
void SendData(uint8_t data);

// Fills the display with the specified color
void Clear(uint16_t color);

// Fills the specified rectangle with the specified color
void FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color);

// Fills <count> rectangles located in the program memory with the same color
void FillRects(const Rect* rects, uint8_t count, uint16_t color);

// Prints one glyph. Parameters:
//   font - font to print with
//   x, y - coordiates to print at
//   code - glyph code to print
//   fgColor - foreground color
//   bgColor - background color
// Returns cursor advance along the X axis to print the next glyph or
// DISPLAY_DOES_NOT_FIT if the symbol does not fit to the screen
uint8_t PrintGlyph(const Font *font, uint8_t x, uint8_t y, uint8_t code, uint16_t fgColor, uint16_t bgColor);

// Prints zero-terminates PROGMEM string with the current font,
// foreground and background colors. Parameters:
//   x, y - coordinates to print at
//   string - pointer to the PROGMEM string to print
// Returns x coordinate where the next symbol can be printed
uint8_t PrintString(uint8_t x, uint8_t y, const char *string);

// Prints specified number of characters stored in RAM with the current font,
// foreground and background colors. Parameters:
//   x, y - coordinates to print at
//   string - RAM pointer to the first symbol to print
//   count - symbol count
// Returns x coordinate where the next symbol can be printed
uint8_t PrintStringRam(uint8_t x, uint8_t y, const char *string, uint8_t count);

// Performs hard delay (by using CPU loops) for the specified number of 10 ms ticks
void HardDelay(uint8_t n10msTicks);

// Display data
var Font const* g_font;
var uint16_t g_fgColor;
var uint16_t g_bgColor;

} // extern "C"

} // namespace display

