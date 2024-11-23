#include "assembler_defines.S"

.global InitWatchdog
.global ShiftRight12, ShiftLeft12, I16ToString, I8ToString
.global GetCurrentSumDiv4M, GetEncoderDelta, GetEncoderKey
.global TemperatureToDisplayX100

// void InitWatchdog();
InitWatchdog:
    wdr
    ldi     R24, BV(WDCE) | BV(WDE)
    sts     (WDTCSR), R24
    
    // 0.125 seconds interval
    ldi     R24, BV(WDE) | BV(WDP1) | BV(WDP0)
    sts     (WDTCSR), R24
    
    ret

; uint16_t ShiftRight12(uint32_t value)
ShiftRight12:
	; R25:R24:R23:R22 = value
    ; R25:R24 = result
	; Shift R25:R24:R23 left by 4 bits, thus we'll have the result in R25:R24
	swap	R25
	andi	R25, 0xF0

	swap	R24
	eor		R25, R24
	andi	R24, 0xF0
	eor		R25, R24

	swap	R23
	andi	R23, 0x0F
	or		R24, R23

	ret

; uint32_t ShiftLeft12(uint16_t value);
ShiftLeft12:
    ; R25:R24 = value
    ; R25:R24:R23:R22 = result
    ; Shift R25:R24:R23 right by 4 bits
    swap    R24
    mov     R23, R24
    andi    R23, 0xF0
    andi    R24, 0x0F

    swap    R25
    eor     R24, R25
    andi    R25, 0x0F
    eor     R24, R25

    clr     R22
    ret

; void I16ToString(uint16_t value, char *buffer, uint8_t maxLeadingSpaces);
I16ToString:
	; R25:R24 = value
	; R23:R22 = buffer
	; R20 = maxLeadingSpaces

	movw	Z, R22

	ldi		R22, lo8(10000)
	ldi		R23, hi8(10000)
	rcall	dtsCount

	ldi		R22, lo8(1000)
	ldi		R23, hi8(1000)
	rcall	dtsCount

	ldi		R22, lo8(100)
	ldi		R23, hi8(100)
	rcall	dtsCount

	ldi		R22, lo8(10)
	ldi		R23, hi8(10)
	rcall	dtsCount

	subi	R24, -'0'
	st		Z, R24
	ret

dtsCount:
	; Instead of using a real division we continiously subtract divisor from
	; the divident here and count how many times we succeed. This takes about
	; <quotient>*5 cycles, which is only 5*5 = 25 for an average digit.
	; Unfortunately this is much faster than any existing division algorithm
	; and even faster then the division-by-multiplication ((x*0xCCCD) >> 19)
	; method which would take about 30 cycles for a digit (since we need to
	; work with 32 bit values there).
	
	; R23:R22 = value to subtract
	ldi		R21, '0'

	sub		R24, R22
	sbc		R25, R23
	brcs	dtsZero

dtsCountLoop:
	inc		R21
	sub		R24, R22
	sbc		R25, R23
	brcc	dtsCountLoop

	add		R24, R22
	adc		R25, R23

	st		Z+, R21
	
	; Prevent using spaces for any successive zero digits
	clr		R20
	ret

dtsZero:
	add		R24, R22
	adc		R25, R23

	dec     R20
    brmi    dtsNoSpace
	ldi		R21, 0x7F

dtsNoSpace:
	st		Z+, R21
	ret

; void I8ToString(uint8_t value, char *buffer);
I8ToString:
    ; R24 = value
    ; R23:R22 = buffer
    movw    Z, R22
    adiw    Z, 3

    ldi     R22, 205
    ldi     R23, 10

    rcall   i8_digit
    rcall   i8_digit
    subi    R24, -'0'
    st      -Z, R24

    clr     R1
    ret

i8_digit:
    mul     R24, R22
    lsr     R1
    lsr     R1
    lsr     R1
    mov     R25, R1

    mul     R25, R23
    sub     R24, R0
    subi    R24, -'0'
    st      -Z, R24

    mov     R24, R25
    ret

; uint32_t GetCurrentSumDiv4M();
GetCurrentSumDiv4M:
    cli
    lds     R21, (g_totalCurrentSum + 2)
    lds     R22, (g_totalCurrentSum + 3)
    lds     R23, (g_totalCurrentSum + 4)
    lds     R24, (g_totalCurrentSum + 5)
    sei

    ; We know that 4 MSBs of g_totalCurrentSum are always zero
    lsl     R21
    rol     R22
    rol     R23
    rol     R24

    lsl     R21
    rol     R22
    rol     R23
    rol     R24

    clr     R25
    ; R25:R24:R23:R22 = g_totalCurrentSum/4M

    ret

; int8_t GetEncoderDelta();
GetEncoderDelta:
    cli
    lds     R24, (g_encoderCounter)
    sts     (g_encoderCounter), R1
    sei
    ret

; EEncoderKey GetEncoderKey();
GetEncoderKey:
    cli
    lds     R24, (g_encoderKey)
    sts     (g_encoderKey), R1
    sei
    ret

; int16_t TemperatureToDisplayX100(int16_t temperture)
TemperatureToDisplayX100:
    ; R25:R24 = temperature
    ; R25:R24 = result
    ; result = temperature*100/256
    ldi     R23, 100
    mul     R24, R23
    movw    R20, R0

    muls    R25, R23
    mov     R22, R1
    clr     R1
    add     R21, R0
    adc     R22, R1
    ; R22:R21:R20 = temperature*100

    mov     R24, R21
    mov     R25, R22
    ret

