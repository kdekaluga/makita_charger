#include "../assembler_defines.S"

.global SendCommand, SendData, Clear, FillRect, FillRects
.global PrintGlyph, PrintString, PrintStringRam
.global HardDelay

; ***
; NOTE: It was found experimentally that the minimum delay between
; the two successive SPI data transfers is at least 18 clocks,
; (including the next OUT command), othewise SPI works unreliably
; ***

; Wait for a previous transfer to finish. Uses R0
.macro SPI_WAIT
	in		R0, (SPSR)
	sbrs	R0, SPIF
	rjmp	.-6
.endm

; Switch D/C to command (0), 2 clocks
.macro SPI_CMD
	cbi		(PORTB), PB_DISPLAY_DC
.endm

; Switch D/C to data (1), 2 clocks
.macro SPI_DATA
	sbi		(PORTB), PB_DISPLAY_DC
.endm

; Send the 'r' register to SPI, 1 clock
.macro SPI_SND r
	out		(SPDR), \r
.endm

; void SendCommand(uint8_t command);
SendCommand:
	SPI_WAIT
	SPI_CMD
	SPI_SND R24
	ret

; void SendData(uint8_t data)
SendData:
	SPI_WAIT
	SPI_DATA
	SPI_SND	R24
	ret

; ***********

delay17c:
	nop
delay16c:
	nop
delay15c:
	nop
delay14c:
	nop
delay13c:
	nop
delay12c:
	nop
delay11c:
	nop
delay10c:
	nop
delay9c:
	nop
delay8c:
	nop
delay7c:
	; 7c = rcall(3) + ret(4)
	ret

; Send the CASET command. R19:R18 = Xstart, R25:R24 = Xend
sendCaset:
	; Must be at least 13c after the last SPI send
	mov		R0, R19
	ldi		R19, DISPLAY_CMD_CASET
	SPI_CMD
	SPI_SND	R19

	rcall	delay14c

sendCasetContinue:
	; 14c
	mov		R19, R0
	SPI_DATA
	SPI_SND R19
	
	rcall	delay17c
	SPI_SND	R18

	rcall	delay17c
	SPI_SND	R25

	; Clear SPIF flag set by the previous transmissions so the
	; next SPI_WAIT macro (if any) won't prematurely exit
	rcall	delay16c
	in		R0, (SPSR)
	SPI_SND	R24

	ret; -> 4c

; Send the RASET command. R19:R18 = Ystart, R25:R24 = Yend
sendRaset:
	; Must be at least 13c after the last SPI send
	mov		R0, R19
	ldi		R19, DISPLAY_CMD_RASET
	SPI_CMD
	SPI_SND	R19

	rcall	delay12c
	rjmp	sendCasetContinue; -> 14c

; ***

; void Clear(uint16_t color);
Clear:
	; R25:R24 = color

	MPUSH	16, 17

	movw	R16, R24
	clr		R24
	clr		R22
	ldi		R20, DISPLAY_WIDTH
	ldi		R18, DISPLAY_HEIGHT
	rcall	FillRect

	MPOP	16, 17
	ret

; ***

;void FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color);
FillRect:
	; R24 = x
	; R22 = y
	; R20 = width
	; R18 = height
	; R17:R16 = color

	; Check that x and y are valid
	cpi		R24, DISPLAY_WIDTH
	brcc	frEmpty
	cpi		R22, DISPLAY_HEIGHT
	brcc	frEmpty

	; Check that width and height > 0
	and		R20, R20
	breq	frEmpty
	and		R18, R18
	breq	frEmpty

	mov		R21, R18

	clr		R19
	mov		R18, R24
	; R19:R18 = Xstart

	clr		R25
	add		R24, R20
	brcs	frLimitWidth
	cpi		R24, DISPLAY_WIDTH + 1
	brcs	frWidthOk
frLimitWidth:
	ldi		R24, DISPLAY_WIDTH
frWidthOk:
	; R25:R24 = Xend + 1

	mov		R23, R24
	sub		R23, R18
	; R23 = actual width

	dec		R24
	rcall	sendCaset
	; 4c

	mov		R18, R22
	; R19:R18 = Ystart

	mov		R24, R22
	add		R24, R21
	; 7c
	brcs	frLimitHeight; -> 9c
	cpi		R24, DISPLAY_HEIGHT + 1
	brcs	frHeightOk; -> 11c
frLimitHeight:
	; 9/10c
	ldi		R24, DISPLAY_HEIGHT
frHeightOk:
	; 10/11c
	mov		R22, R24
	; R22 = R25:R24 = Yend + 1

	dec		R24
	rcall	sendRaset; -> 15/16c
	; 4c

	sub		R22, R18
	; 5c, R22 = actual height
	
	rcall	delay9c
	ldi		R24, DISPLAY_CMD_RAMWR
	SPI_CMD
	; 17c
	SPI_SND	R24

	mul		R23, R22
	movw	R24, R0
	clr		R1

frLoop:
	; 4c
	rcall	delay11c
	SPI_DATA
	; 17c
	SPI_SND	R17
	rcall	delay17c
	SPI_SND	R16

	sbiw	R24, 1
	brne	frLoop; -> 4c

frEmpty:
	ret

;void FillRects(const Rect* rects, uint8_t count, uint16_t color);
FillRects:
	; R25:R24 = rects
	; R22 = count
	; R21:R20 = color

	MPUSH	16, 17

	movw	Z, R24
	movw	R16, R20
	mov		R26, R22

frs_loop:
	subi	R26, 1
	brcs	frs_end

	lpm		R24, Z+
	lpm		R22, Z+
	lpm		R20, Z+
	lpm		R18, Z+
	rcall	FillRect
	rjmp	frs_loop

frs_end:
	MPOP	16, 17
	ret

; ***

pgNoGlyph:
	; Return zero
	clr		R24
	ret

pgDoesNotFit:
	; Return DISPLAY_DOES_NOT_FIT
	pop		R29
	pop		R28
	ldi		R24, DISPLAY_DOES_NOT_FIT
	ret

; uint8_t PrintGlyph(const Font *font, uint8_t x, uint8_t y, uint8_t code, uint16_t fgColor, uint16_t bgColor);
PrintGlyph:
	; R25:R24 = font
	; R22 = x
	; R20 = y
	; R18 = code
	; R17:R16 = fgColor
	; R15:R14 = bgColor

	movw	Z, R24
	lpm		R26, Z+
	lpm		R27, Z+
	; R27:R26 = m_glyphs

	; if (code > m_lastChar)
	lpm		R0, Z+
	; R0 = m_lastChar
	cp		R0, R18
	brcs	pgNoGlyph

	; if (code < m_firstChar)
	lpm		R0, Z+
	; R0 = m_firstChar
	sub		R18, R0
	brcs	pgNoGlyph

	push	R28
	push	R29

	lpm		R28, Z+
	lpm		R29, Z+
	; R28 = m_yFirstLineOffset
	; R29 = m_yAdvance

	mov		ZL, R18
	clr		ZH
	add		ZL, ZL
	adc		ZH, ZH
	add		ZL, ZL
	adc		ZH, ZH
	add		ZL, ZL
	adc		ZH, ZH

	sub		ZL, R18
	sbc		ZH, R1

	add		ZL, R26
	adc		ZH, R27
	; Z = m_glyphs + (code - m_firstChar)*7 = &m_glyphs[code - m_firstChar]

	lpm		R26, Z+
	lpm		R27, Z+
	adiw	R24, 6
	add		R24, R26
	adc		R25, R27
	; R24:R25 = &m_bitmaps[m_offset]

	lpm		R23, Z+
	lpm		R21, Z+
	; R23 = m_width
	; R21 = m_height

	mov		R18, R22
	lpm		R22, Z+
	; R18 = x
	; R22 = m_xAdvance

	lpm		R26, Z+
	lpm		R27, Z+
	; R26 = m_xOffset
	; R27 = m_yOffset

	movw	Z, R24

	; Everything is loaded into the registers:
	; R15:R14 = bgColor
	; R17:R16 = fgColor
	; R18 = x
	; R20 = y
	; R21 = m_height
	; R22 = m_xAdvance
	; R23 = m_width
	; R26 = m_xOffset
	; R27 = m_yOffset
	; R28 = m_yFirstLineOffset
	; R29 = m_yAdvance
	; Z = glyph bitmap address
	; Free volatile registers: R18, R24, R25

	; sendCaset(x, x + m_xAdvance - 1)
	; Don't need some instructions for 240x240 display, they are commented out
	clr		R19
	clr		R25
	mov		R24, R18
	add		R24, R22
	; adc		R25, R1

	; Check whether the symbol fits to the screen
	brcs	pgDoesNotFit
	cpi		R24, DISPLAY_WIDTH + 1
	brcc	pgDoesNotFit
	sbiw	R24, 1
	rcall	sendCaset
	; 4c

	; sendRaset(y + m_yFirstLineOffset, y + m_yFirstLineOffset + m_yAdvance - 1)
	mov		R18, R20
	add		R18, R28
	; adc		R19, R1
	; sbrc	R28, 7
	; dec		R19
	movw	R24, R18
	add		R24, R29
	; adc		R25, R1
	sbiw	R24, 1
	rcall	sendRaset; -> 13c
	; 4c

	lpm		R24, Z+
	ldi		R20, 8
	; 8c
	mov		R25, R22
	sub		R25, R23
	sub		R25, R26
	; R25 = m_xAdvance - m_width - m_xOffset
	; 11c
	nop
	rjmp	.+0
	ldi		R18, DISPLAY_CMD_RAMWR
	; 15c
	SPI_CMD
	SPI_SND	R18

	; x and y are not needed anymore, register map update:
	; R18 -> x counter
	; R19 -> y counter
	; R20 = bit counter (initially 8)
	; R24 = glyph bits
	; R25 = m_xAdvance - m_width - m_xOffset

	; *** Fill lines above the glyph with the backgroup color ***

	; How many top empty lines do we have?
	sub		R27, R28
	; R27 = m_yOffset - m_yFirstLineOffset = top empty lines count

	mov		R19, R27
	; R19 = top empty lines count, it can be zero (but not less)
	breq	pgNoTopBgLines; -> 4c
	; 3c
	rjmp	pgFillTopYLoop; -> 5c

pgFillTopXLoopDelay:
	; 3c
	nop
	rjmp	pgFillTopXLoop; -> 6c

pgFillTopYLoop:
	; 5c
	mov		R18, R22
	; R18 = m_xAdvance

pgFillTopXLoop:
	; 6c
	rcall	delay9c
	SPI_DATA
	SPI_SND	R15
	rcall	delay17c
	SPI_SND	R14

	dec		R18
	brne	pgFillTopXLoopDelay; -> 3c
	; 2c
	dec		R19
	brne	pgFillTopYLoop; -> 5c

pgNoTopBgLines:
	; 4c

	; *** Draw the glyph ***
	; There are 3 draw stages:
	; 1. Prefill with BG color, m_xOffset pixels (can be 0)
	; 2. Draw the glyph line, m_width pixels (cannot be 0)
	; 3. Postfill with BG color, (m_xAdvance - m_width - m_xOffset) pixels (R25, can be 0)

	mov		R19, R21
	; R19 = m_height, can be 0

	and		R19, R19
	breq	pgFillBottom; -> 8c

pgPrefill:
	; 7c
	mov		R18, R26
	; R18 = m_xOffset, can be 0

pgPrefillLoop:
	; 8c
	and		R18, R18
	breq	pgNoPrefill; -> 11c
	; 10c
	nop
	rjmp	.+0
	rjmp	.+0
	; 15c

	SPI_DATA
	SPI_SND	R15
	rcall	delay17c
	SPI_SND	R14

	dec		R18
	nop
	rjmp	.+0
	rjmp	.+0
	; 6c
	rjmp	pgPrefillLoop; -> 8c

pgNoPrefill:
	; 11c
	mov		R18, R23
	; R18 = m_width, > 0

pgFillLoop:
	; 12c
	lsl		R24
	brcs	pgFillPixel; -> 15c
	nop
	; 15c, BG color pixel
	SPI_DATA
	SPI_SND	R15
	rcall	pgFillLoadByteIfNeeded
	SPI_SND	R14

	dec		R18
	brne	pgFillContinue; -> 3c
	; 2c
	and		R25, R25
	brne	pgPostFill; -> 5c
	; 4c
	dec		R19
	brne	pgPrefill; -> 7c
	; 6c
	rjmp	pgFillBottom; -> 8c

pgFillPixel:
	; 15c, FG color pixel
	SPI_DATA
	SPI_SND	R17
	rcall	pgFillLoadByteIfNeeded
	SPI_SND	R16

	dec		R18
	brne	pgFillContinue; -> 3c
	; 2c
	and		R25, R25
	brne	pgPostFill; -> 5c

pgPostFillEnd:
	; 4c
	dec		R19
	brne	pgPrefill; -> 7c
	; 6c
	rjmp	pgFillBottom; -> 8c

pgFillContinue:
	; 3c
	rcall	delay7c
	rjmp	pgFillLoop; -> 12c

pgPostFill:
	; 5c
	mov		R18, R25
	rcall	delay11c

pgPostFillLoop:	
	; 17c
	SPI_SND	R15
	rcall	delay17c
	SPI_SND	R14

	nop
	dec		R18
	breq	pgPostFillEnd; -> 4c
	; 3c
	rcall	delay12c
	rjmp	pgPostFillLoop; -> 17c

pgFillBottom:
	; 8c
	mov		R19, R29
	sub		R19, R27
	sub		R19, R21
	; 11c, R19 = bottom empty lines count

	breq	pgDone; -> 13 c

pgFillBottomLoopY:
	; 12c
	mov		R18, R22
	; R18 = m_xAdvance

pgFillBottomLoopX:
	; 13c
	rjmp	.+0
	SPI_DATA
	SPI_SND	R15
	rcall	delay17c
	SPI_SND	R14

	rcall	delay7c
	dec		R18
	brne	pgFillBottomContinue; -> 10c
	; 9c
	dec		R19
	brne	pgFillBottomLoopY; -> 12c
	
	rjmp	.+0

pgDone:
	; 13c, just in case
	mov		R24, R22
	; R24 = m_xAdvance

	pop		R29
	pop		R28
	ret

pgFillBottomContinue:
	; 10c
	nop
	rjmp	pgFillBottomLoopX; -> 13c


pgFillLoadByteIfNeeded:
	; 3c
	dec		R20
	brne	pgNoLoad; -> 6c
	; 5c

	lpm		R24, Z+
	ldi		R20, 8
	; 9c

	rjmp	delay9c; -> 11c

pgNoLoad:
	; 6c
	rjmp	delay12c; -> 8c
	
; ***

;uint8_t PrintString(uint8_t x, uint8_t y, const char *string);
PrintString:
	; R24 = x
	; R22 = y
	; R21:R20 = string

	MPUSH	12, 17
	MPUSH	28, 29

	movw	R28, R20
	mov		R12, R24
	mov		R13, R22
	lds		R14, (g_bgColor + 0)
	lds		R15, (g_bgColor + 1)
	lds		R16, (g_fgColor + 0)
	lds		R17, (g_fgColor + 1)
	; Y = string
	; R12 = x
	; R13 = y
	; R15:R14 = g_bgColor
	; R17:R16 = g_fgColor

psLoop:
	movw	Z, Y
	lpm		R18, Z+
	movw	Y, Z

	and		R18, R18
	breq	psRet

	lds		R24, (g_font + 0)
	lds		R25, (g_font + 1)
	mov		R22, R12
	mov		R20, R13
	rcall	PrintGlyph

	cpi		R24, DISPLAY_DOES_NOT_FIT
	breq	psRet

	; x += PrintGlyph()
	add		R12, R24
	rjmp	psLoop

psRet:
	; return x
	mov		R24, R12

	MPOP	28, 29
	MPOP	12, 17

psrNoSymbols:
	ret

; ***

; uint8_t PrintStringRam(uint8_t x, uint8_t y, const char *string, uint8_t count);
PrintStringRam:
	; R24 = x
	; R22 = y
	; R21:R20 = string
	; R18 = count

	and		R18, R18
	breq	psrNoSymbols

	; Special push order to use the return code from PrintString
	MPUSH	12, 17
	MPUSH	28, 29
	push	R11

	movw	R28, R20
	mov		R11, R18
	mov		R12, R24
	mov		R13, R22
	lds		R14, (g_bgColor + 0)
	lds		R15, (g_bgColor + 1)
	lds		R16, (g_fgColor + 0)
	lds		R17, (g_fgColor + 1)
	; Y = string
	; R11 = count
	; R12 = x
	; R13 = y
	; R15:R14 = g_bgColor
	; R17:R16 = g_fgColor

psrLoop:
	ld		R18, Y+

	lds		R24, (g_font + 0)
	lds		R25, (g_font + 1)
	mov		R22, R12
	mov		R20, R13
	rcall	PrintGlyph

	cpi		R24, DISPLAY_DOES_NOT_FIT
	breq	psrRet

	add		R12, R24
	dec		R11
	brne	psrLoop

psrRet:
	pop		R11
	rjmp	psRet

; ***

; void HardDelay(uint8_t n10msTicks)
HardDelay:
	; R24 = n10msTicks

hd_loop:
	ldi		R26, lo8(10000)
	ldi		R27, hi8(10000)

	; Each loop iteration is 16 cycles or 1 usec.
	; Thus 10K iterations is 10 ms
hd_loop1us:
	rcall	delay12c
	sbiw	R26, 1
	brne	hd_loop1us

	dec		R24
	brne	hd_loop

	ret

