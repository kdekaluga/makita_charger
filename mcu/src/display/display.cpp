#include "../includes.h"

// Since font headers instantiate data, we can include them only once. Do it here
#include "fonts/FreeSans12.h"
#include "fonts/FreeSans18.h"

namespace display {

const char pm_warning[] PROGMEM = "WARNING";
const char pm_error[] PROGMEM = "ERROR";
const char pm_failure[] PROGMEM = "FAILURE";
const char pm_lowPower[] PROGMEM = "Low source\nvoltage! Operation\nsuspended until\npower restoration.";
const char pm_overvoltage[] PROGMEM = "Output overvoltage\ndetected!\n"
    "This could be\nan internal failure.\nClick OK to reset";
const char pm_overcurrent[] PROGMEM = "Output overcurrent\ndetected!\n"
    "This could be\nan internal failure.\nClick OK to reset";
const char pm_ok[] PROGMEM = "OK";
const char pm_yes[] PROGMEM = "YES";
const char pm_no[] PROGMEM = "NO";

void Init()
{
    // Reset ST7789
    PORTB &= ~BV(PB_DISPLAY_RES);

    // While ST7789 is under reset, out a dummy byte to the SPI so
    // the SPI ready flag will be set for the next SendCommand/SendData function
    SPDR = 0;

    utils::Delay(2);
    PORTB |= BV(PB_DISPLAY_RES);
    utils::Delay(13);

    SendCommand(DISPLAY_CMD_SLPOUT);
    utils::Delay(2);

    SendCommand(DISPLAY_CMD_COLMOD);
    SendData(0x55);

    SendCommand(DISPLAY_CMD_INVON);
    SendCommand(DISPLAY_CMD_NORON);
    SendCommand(DISPLAY_CMD_DISPON);
}

void __attribute__((noinline)) SetSans12()
{
    g_font = &FreeSans12::g_font;
}

void __attribute__((noinline)) SetSans18()
{
    g_font = &FreeSans18::g_font;
}

const Font* GetSans12()
{
    return &FreeSans12::g_font;
}

const Font* GetSans18()
{
    return &FreeSans18::g_font;
}

void SetBgColor(uint16_t bgColor)
{
    g_bgColor = bgColor;
}

void SetColor(uint16_t color)
{
    g_fgColor = color;
}

void SetColors(uint16_t bgColor, uint16_t fgColor)
{
    g_bgColor = bgColor;
    g_fgColor = fgColor;
}

void __attribute__((noinline)) SetUiElementColors(int8_t cursorPosition, int8_t nElement)
{
    uint16_t bgColor = CLR_BLACK, fgColor = CLR_WHITE;
    if ((cursorPosition & (~DSD_CURSOR_SKIP)) == nElement)
    {
        bgColor = CLR_BG_CURSOR;
        if (cursorPosition & DSD_CURSOR_SKIP)
            fgColor = CLR_BG_CURSOR;
    }

    g_bgColor = bgColor;
    g_fgColor = fgColor;
}

uint8_t PrintGlyph(uint8_t x, uint8_t y, uint8_t code)
{
    return x + PrintGlyph(g_font, x, y, code, g_fgColor, g_bgColor);
}

// Returns width of the specified character using the selected font
uint8_t GetCharWidth(uint8_t c)
{
    uint8_t firstChar = pgm_read_byte(&g_font->m_firstChar);
    uint8_t lastChar = pgm_read_byte(&g_font->m_lastChar);
    if (c < firstChar || c > lastChar)
        return 0;

    const Glyph* glyph = reinterpret_cast<const Glyph*>(pgm_read_word(&g_font->m_glyphs)) + (c - firstChar);
    return pgm_read_byte(&glyph->m_xAdvance);
}

// Return width of the specified string (located in program memory) using the selected font
uint8_t GetTextWidth(const char* text)
{
    uint8_t width = 0;
    while (uint8_t c = pgm_read_byte(text++))
        width += GetCharWidth(c);

    return width;
}

uint8_t GetTextWidthRam(const char* text, uint8_t length)
{
    uint8_t width = 0;
    while (length-- > 0)
        width += GetCharWidth(*text++);

    return width;
}

uint8_t DrawSettableDecimal(uint8_t x, uint8_t y, uint8_t count, uint8_t cursorPos, uint16_t fgColor, uint16_t bgColor)
{
    bool hideSelected = (cursorPos & DSD_CURSOR_SKIP);
    cursorPos &= 0x7F;

    for (uint8_t i = 0; i < count; ++i)
    {
        uint16_t bgColorDraw = bgColor;
        uint16_t fgColorDraw = fgColor;
        if (g_buffer[i] != '.' && !cursorPos--)
        {
            bgColorDraw = CLR_BG_CURSOR;
            if (hideSelected)
                fgColorDraw = CLR_BG_CURSOR;
            else
                fgColorDraw = CLR_FG_CURSOR;
        }

        x += PrintGlyph(g_font, x, y, g_buffer[i], fgColorDraw, bgColorDraw);
    }
    return x;
}

// *** Message box ***

uint8_t MessageBox(const char* caption, const char* text, uint8_t flags)
{
    // Message box looks like:
    // +------------------+
    // |     CAPTION      |
    // +------------------+
    // | First text line, |
    // |   second one.    |
    // |                  |
    // |     YES   NO     |
    // +------------------+

    // No need to use any other font...
    SetSans12();

    constexpr uint8_t lineThick = 2;
    constexpr uint8_t captionYMargin = 3;
    constexpr uint8_t margin = 10;
    constexpr uint8_t maxTextWidth = 220 - margin*2;
    constexpr uint8_t minTextWidth = 150 - margin*2;
    constexpr uint8_t buttonDistance = 15;
    constexpr uint8_t buttonWidth = 60;

    const uint8_t captionWidth = GetTextWidth(caption);
    uint8_t textWidth = (captionWidth < minTextWidth ? minTextWidth : captionWidth);
    uint8_t nLines = 1;

    uint8_t currentWidth = 0;
    for (const char *p = text; const uint8_t c = pgm_read_byte(p); ++p)
    {
        uint8_t charWidth = GetCharWidth(c);
        if (c == '\n' || currentWidth + charWidth > maxTextWidth)
        {
            if (currentWidth > textWidth)
                textWidth = currentWidth;

            ++nLines;
            currentWidth = 0;
        }

        currentWidth += charWidth;
    }

    const uint8_t lineHeight = pgm_read_byte(&g_font->m_yAdvance);
    const int8_t lineYOffset = pgm_read_byte(&g_font->m_yFirstLineOffset);
    const uint8_t textHeight = lineHeight*nLines;
    const uint8_t mbWidth = textWidth + margin*2 + lineThick*2;
    const uint8_t captionHeight = lineHeight + captionYMargin*2;
    uint8_t mbHeight = captionHeight + textHeight + margin*2 + lineThick;
    if (flags & (MB_OK | MB_YESNO))
        mbHeight += margin*2 + lineHeight;

    uint16_t fillColor = CLR_BLUE;
    if (flags & MB_ERROR)
        fillColor = RGB(237, 20, 91);
    else if (flags & MB_WARNING)
        fillColor = RGB(0xD0, 0xD0, 0);

    uint8_t x = (DISPLAY_WIDTH - mbWidth)/2;
    uint8_t y = (DISPLAY_HEIGHT - mbHeight)/2;

    // Caption
    FillRect(x, y, mbWidth, captionHeight, fillColor);
    g_bgColor = fillColor;
    g_fgColor = CLR_WHITE;
    PrintString((DISPLAY_WIDTH - captionWidth)/2, y + captionYMargin - lineYOffset + 1, caption);

    // Fill body, draw lines
    y += captionHeight;
    uint8_t bodyHeight = mbHeight - captionHeight - lineThick;
    FillRect(x + lineThick, y, mbWidth - lineThick*2, bodyHeight, CLR_BLACK);
    FillRect(x, y, lineThick, bodyHeight, fillColor);
    FillRect(x + mbWidth - lineThick, y, lineThick, bodyHeight, fillColor);
    FillRect(x, y + bodyHeight, mbWidth, lineThick, fillColor);

    // Body text
    y += margin;

    // Loop over all text lines    
    for (;;)
    {
        // Calculate line width and symbol count
        const char* p = text;
        uint8_t c;
        uint8_t lineWidth = 0;
        uint8_t nChars = 0;
        for (;;)
        {
            c = pgm_read_byte(p++);
            if (!c || c == '\n')
                break;

            uint8_t charWidth = GetCharWidth(c);
            if (lineWidth + charWidth > maxTextWidth)
            {
                --p;
                break;
            }

            lineWidth += charWidth;
            ++nChars;
        }

        // Print line
        x = (DISPLAY_WIDTH - lineWidth)/2;
        while(nChars--)
            x += PrintGlyph(g_font, x, y - lineYOffset, pgm_read_byte(text++), CLR_WHITE, CLR_BLACK);

        text = p;
        y += lineHeight;

        if (!c)
            break;
    }

    uint8_t nButtons = flags & (MB_OK | MB_YESNO);
    if (!nButtons)
        return 0;
    
    y += margin;
    int8_t nSelectedButton = (flags & MB_DEFAULT_NO) >> 2;

    const auto DrawButton = [&](uint8_t x, const char* text, bool bSelected)
    {
        uint16_t bgColor;
        if (bSelected)
        {
            bgColor = CLR_BLUE;
            g_fgColor = CLR_WHITE;
        }
        else
        {
            bgColor = CLR_BG_INACTIVE;
            g_fgColor = CLR_BLACK;
        }
        g_bgColor = bgColor;
        
        uint8_t width = GetTextWidth(text);
        uint8_t offset = (buttonWidth - width)/2;
        FillRect(x, y, offset, lineHeight, bgColor);
        PrintString(x + offset, y - lineYOffset, text);
        FillRect(x + offset + width, y, buttonWidth - offset - width, lineHeight, bgColor);
    };

    for (;;)
    {
        // TODO: redraw the entire message box if ProcessFailureStates() returns true
        ProcessFailureStates();

        if (nButtons == 1)
        {
            // Just "OK", always under cursor
            DrawButton((DISPLAY_WIDTH - buttonWidth )/2, pm_ok, true);
        }
        else
        {
            // "YES" and "NO"
            x = (DISPLAY_WIDTH - buttonWidth*2 - buttonDistance)/2;
            DrawButton(x, pm_yes, nSelectedButton == 0);
            x += buttonWidth + buttonDistance;
            DrawButton(x, pm_no, nSelectedButton == 1);

            nSelectedButton += utils::GetEncoderDelta();
            if (nSelectedButton < 0)
                nSelectedButton = 0;
            else if (nSelectedButton > 1)
                nSelectedButton = 1;
        }

        if (utils::GetEncoderKey() == EEncoderKey::Up)
            return nSelectedButton;
    }
}

bool ProcessFailureStates()
{
    bool result = false;
    for (;;)
    {
        if (g_failureState & FAILURE_NONE)
        {
            if (g_resetDisplay)
            {
                Init();
                g_resetDisplay = false;
                return true;
            }

            return result;
        }

        if (g_failureState & FAILURE_POWER_LOW)
        {
            // Try to save settings first
            g_settings.SaveToEeprom();

            display::MessageBox(display::pm_warning, pm_lowPower, MB_WARNING);

            while (g_failureState & FAILURE_POWER_LOW)
                asm volatile ("sleep");

            utils::ClearPendingKeys();
            result = true;
            continue;
        }

        if (g_failureState & FAILURE_OVERVOLTAGE)
        {
            display::MessageBox(display::pm_failure, pm_overvoltage, MB_ERROR | MB_OK);

            cli();
            g_failureState &= ~(FAILURE_OVERVOLTAGE);
            sei();
            result = true;
            continue;
        }

        if (g_failureState & FAILURE_OVERCURRENT)
        {
            display::MessageBox(display::pm_failure, pm_overcurrent, MB_ERROR | MB_OK);

            cli();
            g_failureState &= ~(FAILURE_OVERCURRENT);
            sei();
            result = true;
            continue;
        }

        asm volatile ("sleep");
    }
}

// *** UI Screen ***

int8_t UiScreen::DrawBackground() const
{
    UiDrawBackgroundFunc func = reinterpret_cast<UiDrawBackgroundFunc>(pgm_read_word(&m_drawBackgroundFunc));
    return func();
}

void UiScreen::DrawElements(int8_t cursorPosition, uint8_t ticksElapsed) const
{
    UiDrawElementsFunc func = reinterpret_cast<UiDrawElementsFunc>(pgm_read_word(&m_drawElementsFunc));
    func(cursorPosition, ticksElapsed);
}

bool UiScreen::OnClickElement(int8_t cursorPosition) const
{
    UiOnClickElementFunc func = reinterpret_cast<UiOnClickElementFunc>(pgm_read_word(&m_onClickElementFunc));
    return func(cursorPosition);
}

void UiScreen::OnChangeElement(int8_t cursorPosition, int8_t delta) const
{
    UiOnChangeElementFunc func = reinterpret_cast<UiOnChangeElementFunc>(pgm_read_word(&m_onChangeElementFunc));
    func(cursorPosition, delta);
}

bool UiScreen::OnLongClick(int8_t cursorPosition) const
{
    UiOnLongClickFunc func = reinterpret_cast<UiOnLongClickFunc>(pgm_read_word(&m_onLongClickFunc));
    return func(cursorPosition);
}

void UiScreen::Show() const
{
    g_encoderCounter = 0;

    int8_t cursorPosition = DrawBackground();
    int8_t elementCount = static_cast<int8_t>(pgm_read_byte(&m_elementCount));
    uint8_t tick100Hz = g_100HzCounter;
    uint16_t ticks = 0;
    bool bEditMode = false;
    for (;;)
    {
        uint8_t newTick100Hz = g_100HzCounter;
        uint8_t dt = newTick100Hz - tick100Hz;
        tick100Hz = newTick100Hz;
        ticks += dt;

        if (ProcessFailureStates())
        {
            DrawBackground();
            tick100Hz = g_100HzCounter;
            ticks = 0;
        }

        DrawElements(cursorPosition | (bEditMode && (ticks & 0x20) ? DSD_CURSOR_SKIP : 0), dt);

        if (ticks > 1000)
        {
            cursorPosition |= DSD_CURSOR_HIDDEN;
            bEditMode = false;
        }

        // Process encoder key
        EEncoderKey key = utils::GetEncoderKey();
        if (key == EEncoderKey::Up)
        {
            // If cursor was hidden, just show it, do nothing more
            if (cursorPosition & DSD_CURSOR_HIDDEN)
                cursorPosition &= ~DSD_CURSOR_HIDDEN;
            
            // Otherwise call the click function. If it returns true, switch the edit mode on/off
            else if (OnClickElement(cursorPosition))
                bEditMode = !bEditMode;

            tick100Hz = g_100HzCounter;
            ticks = 0x20;
        }
        else if (key == EEncoderKey::UpLong)
        {
            if (OnLongClick(cursorPosition))
            {
                // Switch output off and save settings before exiting
                g_outOn = false;
                g_pidTargetVoltage = g_pidTargetCurrent = 0;
                g_settings.SaveToEeprom();

                utils::Delay(2);
                return;
            }

            bEditMode = false;
            tick100Hz = g_100HzCounter;
            ticks = 0;
        }

        int8_t delta = utils::GetEncoderDelta();
        if (!delta)
            continue;

        ticks = 0;
        if (bEditMode)
        {
            OnChangeElement(cursorPosition, delta);
            continue;
        }

        // If the cursor was hidden, skip one encoder step, so the cursor could appear
        // exactly at the same place where it was before
        if (cursorPosition & DSD_CURSOR_HIDDEN)
        {
            cursorPosition &= ~DSD_CURSOR_HIDDEN;
            delta += (delta > 0 ? -1 : 1);
        }

        cursorPosition += delta;
        while (cursorPosition < 0)
            cursorPosition += elementCount;
        while (cursorPosition >= elementCount)
            cursorPosition -= elementCount;
    }    
}

// *** Draw objects ***

void DrawObjects(const uint8_t* objects, uint16_t bgColor, uint16_t fgColor)
{
    for (;;)
    {
        uint8_t command = pgm_read_byte(objects++);
        uint8_t param = command & DRO_PARAMMASK;
        command &= DRO_CMDMASK;

        switch (command)
        {
        case DRO_END:
            return;

        case _DRO_BGCOLOR:
            bgColor = pgm_read_word(objects);
            objects += 2;
            break;

        case _DRO_FGCOLOR:
            fgColor = pgm_read_word(objects);
            objects += 2;
            break;

        case DRO_FILLRECT:
            for (uint8_t i = 0; i < param; ++i)
            {
                uint8_t x = pgm_read_byte(objects++);
                uint8_t y = pgm_read_byte(objects++);
                uint8_t w = pgm_read_byte(objects++);
                uint8_t h = pgm_read_byte(objects++);
                FillRect(x, y, w, h, bgColor);
            }
            break;

        case _DRO_STRING_S:
        case _DRO_STRING_L:
            {
                const Font* font = (command == _DRO_STRING_S ? &FreeSans12::g_font : &FreeSans18::g_font);
                uint8_t x = pgm_read_byte(objects++);
                uint8_t y = pgm_read_byte(objects++);
                for (uint8_t i = 0; i < param; ++i)
                    x += PrintGlyph(font, x, y, pgm_read_byte(objects++), fgColor, bgColor);
            }
            break;
        }
    }
}

// *** Menu ***

uint8_t Menu::GetItemCount() const
{
    FuncGetItemCount func = reinterpret_cast<FuncGetItemCount>(pgm_read_word(&m_getItemCount));
    if (func)
        return func();

    return pgm_read_byte(&m_itemCount);
}

uint8_t Menu::DrawItem(uint8_t x, uint8_t y, uint8_t nItem) const
{
    FuncDrawItem func = reinterpret_cast<FuncDrawItem>(pgm_read_word(&m_drawItem));
    if (func)
        return func(x, y, nItem);

    return PrintString(x, y, reinterpret_cast<const char*>(pgm_read_word(&m_itemNames[nItem])));    
}

uint8_t Menu::GetItemWidth(uint8_t nItem) const
{
    FuncGetItemWidth func = reinterpret_cast<FuncGetItemWidth>(pgm_read_word(&m_getItemWidth));
    if (func)
        return func(nItem);

    return GetTextWidth(reinterpret_cast<const char*>(pgm_read_dword(&m_itemNames[nItem])));
}

uint8_t Menu::Show() const
{
    return Show(0);
}

uint8_t Menu::Show(uint8_t selectedItem) const
{
    SetSans12();
    display::FillRect(0, 0, 240, 30, CLR_RED_BEAUTIFUL);
    display::SetColors(CLR_RED_BEAUTIFUL, CLR_WHITE);
    display::PrintString(8, 23, reinterpret_cast<const char*>(pgm_read_word(&m_title)));

    constexpr uint8_t yFirst = 59;

    uint8_t currentPageNumber = selectedItem/7;
    uint8_t pageItemWidth = 0;
    uint8_t itemCount = GetItemCount();
    uint8_t pageCount = (itemCount + 6)/7;

    const auto DrawItem = [&](uint8_t nItem)
    {
        SetBgColor(nItem == selectedItem ? CLR_BG_CURSOR : CLR_BLACK);

        uint8_t x = (240 - pageItemWidth)/2;
        uint8_t y = yFirst + (nItem - currentPageNumber*7)*27;
        /*
        utils::I8ToString(nItem + 1, g_buffer);
        g_buffer[3] = '.';
        g_buffer[4] = ' ';
        if (g_buffer[1] == '0')
            g_buffer[1] = 127;

        x = PrintStringRam(x, y, g_buffer + 1, 4);
        */
        x = this->DrawItem(x, y, nItem);
        FillRect(x, y - 21, 120 + pageItemWidth/2 - x, 27, g_bgColor);
    };

    const auto CalcPageItemWidth = [&]()
    {
        uint8_t itemWidth = 0;
        for (uint8_t i = 0; i < 7; ++i)
        {
            uint8_t nItem = currentPageNumber*7 + i;
            if (nItem >= itemCount)
                break;

            uint8_t width = GetItemWidth(nItem);
            if (width > itemWidth)
                itemWidth = width;
        }

        return itemWidth; // + 2*13 + 6 + 6;
    };

    const auto DrawPage = [&]()
    {
        g_buffer[0] = '1' + currentPageNumber;
        g_buffer[1] = '/';
        g_buffer[2] = '0' + pageCount;
        g_bgColor = CLR_RED_BEAUTIFUL;
        PrintStringRam(240 - 7 - 2*13 - 7, 23, g_buffer, 3);
        
        FillRect(0, 30, 240, 210, CLR_BLACK);
        pageItemWidth = CalcPageItemWidth();

        for (uint8_t i = 0; i < 7; ++i)
        {
            uint8_t nItem = currentPageNumber*7 + i;
            if (nItem >= itemCount)
                break;
            
            DrawItem(nItem);
        }
    };

    DrawPage();
    for (;;)
    {
        if (ProcessFailureStates())
        {
            DrawPage();
            continue;
        }

        EEncoderKey key = utils::GetEncoderKey();
        if (key == EEncoderKey::Up)
            return selectedItem;

        int8_t delta = utils::GetEncoderDelta();
        if (!delta)
        {
            asm volatile ("Sleep");
            continue;
        }

        uint8_t oldSelectedItem = selectedItem;
        selectedItem += delta;
        if (selectedItem >= 128)
            selectedItem = 0;

        if (selectedItem >= itemCount)
            selectedItem = itemCount - 1;
        
        uint8_t pageNumber = selectedItem/7;
        if (pageNumber != currentPageNumber)
        {
            currentPageNumber = pageNumber;
            DrawPage();
            continue;
        }

        DrawItem(oldSelectedItem);
        DrawItem(selectedItem);
    }
}



} // namespace display

