; Timer0 interrupt assembly handler and related routines

#include "assembler_defines.S"

.global TIMER0_OVF_vect

tm0_process_current:
    ; Start the voltage channel conversion
    ldi     R19, ADC_CHANNEL_VOLTAGE
    sts     (ADMUX), R19
    ldi     R19, ADC_SRA_VALUE
    sts     (ADCSRA), R19

    ; Check for a short circuit
    ; Here we consider ~9 A and above (0x300 at ADC) as short circuit
    cpi     R17, 3
    brcs    tm0_no_short_circuit

    ; Short circuit detected! Reset the PWM and PID integral values
    clr     R19
    out     (OCR0A), R19
    sts     (g_pwmValue + 0), R19
    sts     (g_pwmValue + 1), R19
    sts     (g_pidIntegral + 0), R19
    sts     (g_pidIntegral + 1), R19
    sts     (g_pidIntegral + 2), R19

tm0_no_short_circuit:
    ; Update the current accumulator
    lds     R19, (g_adcCurrentAcc + 0)
    add     R16, R19
    lds     R19, (g_adcCurrentAcc + 1)
    adc     R17, R19
    sts     (g_adcCurrentAcc + 0), R16
    sts     (g_adcCurrentAcc + 1), R17
    rjmp    tm0_1


TIMER0_OVF_vect:
	push	R16
	in		R16, (SREG)
	push    R16
	push    R17
    push    R18
    push    R19
    ; 11c

    ; Algorithm: we start the voltage value conversion on even interrupts and
    ; the current value conversion on the odd ones. Then we sum up four values
    ; of each kind (totally 8 interrupts) and launch the PID routine to adjust
    ; the PWM width
    lds     R18, (g_timerCounter)
    lds     R17, (g_pwmValue + 1)

    ; We always start ADC conversions manually, thus they are completely alignled
    ; with the PWM generation. This makes measurements more precise, but it was
    ; found expretimentally that if we start a conversion near the point when the PWM
    ; switch is turned off, we got some slight error (~20 ADC units). It happens around
    ; ~6.8 V at the output and we can definitely live with it, but it was decided to
    ; improve the measurement logic to avoid starting a conversion at the aforementioned
    ; moment.
    ;
    ; So now we have two similar sections in the code - the standard one which starts
    ; a conversion after ~43 clocks from the interrupt start and the early measurement
    ; one which starts a conversion after ~26 clocks from the interrupt start. The time
    ; difference between them is 17 CPU clocks or a little bit more than one ADC cycle.
    ;
    ; Finally we check when the PWM switch is turned off by analyzing the OCR0A value
    ; and choose one of the measurement logics to avoid measurement disturbance.
    ;
    ; Experimentally found good range: 55 - 75
    cpi     R17, 65
    brcc    tm0_early_start

    ; Perform PWM dithering to increase its resolution
    lds     R16, (g_pwmValue + 0)
    lds     R19, (g_pwmLowPos)
    add     R16, R19
    ldi     R19, 0
    adc     R17, R19
    out     (OCR0A), R17
    sts     (g_pwmLowPos), R16

    ; if the target PWM is 0, disable it completely 
    ldi     R17, BV(COM0A1) | BV(COM0B1) | BV(WGM01) | BV(WGM00)
    brne    .+2
    ldi     R17, BV(COM0B1) | BV(WGM01) | BV(WGM00)
    out     (TCCR0A), R17

    ; Read results of the previous ADC conversion
    lds     R16, (ADCL)
    lds     R17, (ADCH)

    sbrc    R18, 5
    rjmp    tm0_process_current

    ; Start the current channel conversion
    ldi     R19, ADC_CHANNEL_CURRENT
    sts     (ADMUX), R19
    ldi     R19, ADC_SRA_VALUE
    sts     (ADCSRA), R19
    ; 43c

    ; Update the voltage accumulator
    lds     R19, (g_adcVoltageAcc + 0)
    add     R16, R19
    lds     R19, (g_adcVoltageAcc + 1)
    adc     R17, R19
    sts     (g_adcVoltageAcc + 0), R16
    sts     (g_adcVoltageAcc + 1), R17
    rjmp    tm0_1


; Eraly measurement version
tm0_early_start:
    ; 18c
    ; R17 = g_pwmValue + 1
    ; R18 = g_timerCounter
    sbrc    R18, 5
    rjmp    tm0_early_process_current

    ; Start the current channel conversion
    ldi     R19, ADC_CHANNEL_CURRENT
    sts     (ADMUX), R19
    ldi     R19, ADC_SRA_VALUE
    sts     (ADCSRA), R19
    ; 26c

    ; Voltage
    lds     R19, (g_adcVoltageAcc + 0)
    lds     R16, (ADCL)
    add     R19, R16
    lds     R16, (ADCH)
    sts     (g_adcVoltageAcc + 0), R19
    lds     R19, (g_adcVoltageAcc + 1)
    adc     R19, R16
    sts     (g_adcVoltageAcc + 1), R19

    rjmp    tm0_early_dither

tm0_early_process_current:
    ; Start the voltage channel conversion
    ldi     R19, ADC_CHANNEL_VOLTAGE
    sts     (ADMUX), R19
    ldi     R19, ADC_SRA_VALUE
    sts     (ADCSRA), R19

    ; Current
    lds     R19, (g_adcCurrentAcc + 0)
    lds     R16, (ADCL)
    add     R19, R16
    lds     R16, (ADCH)
    sts     (g_adcCurrentAcc + 0), R19
    lds     R19, (g_adcCurrentAcc + 1)
    adc     R19, R16
    sts     (g_adcCurrentAcc + 1), R19

    cpi     R16, 3
    brcs    tm0_early_dither

    clr     R19
    out     (OCR0A), R19
    sts     (g_pwmValue + 0), R19
    sts     (g_pwmValue + 1), R19
    sts     (g_pidIntegral + 0), R19
    sts     (g_pidIntegral + 1), R19
    sts     (g_pidIntegral + 2), R19

tm0_early_dither:
    ; R17 = g_pwmValue + 1
    lds     R16, (g_pwmValue + 0)
    lds     R19, (g_pwmLowPos)
    add     R16, R19
    ldi     R19, 0
    adc     R17, R19
    out     (OCR0A), R17
    sts     (g_pwmLowPos), R16

    ; OCR0A here cannot be 0, so always enable PWM (-2 clocks)
    ldi     R17, BV(COM0A1) | BV(COM0B1) | BV(WGM01) | BV(WGM00)
    out     (TCCR0A), R17

tm0_1:
    ; Increment the interrupt number
    subi    R18, -0x20
    sts     (g_timerCounter), R18
    breq    tm0_8th

tm0_ret:
    pop     R19
    pop     R18
    pop     R17
	pop		R16
	out		(SREG), R16
	pop		R16

	reti

// ***

tm0_pid_off:
    rjmp    pwm_underflow

tm0_8th:
    ; We are serving the eighth interrupt
    push    R20
    push    R21
    
    ; *** Voltage and current 256-sample averager ***
    lds     R16, (g_adcCurrentAcc + 0)
    lds     R17, (g_adcCurrentAcc + 1)
    lds     R18, (g_adcVoltageAcc + 0)
    lds     R19, (g_adcVoltageAcc + 1)
    ; R17:R16 = g_adcCurrentAcc
    ; R19:R18 = g_adcVoltageAcc

    clr     R20
    lds     R21, (g_adcAveragerVoltageAcc + 0)
    add     R21, R18
    sts     (g_adcAveragerVoltageAcc + 0), R21
    lds     R21, (g_adcAveragerVoltageAcc + 1)
    adc     R21, R19
    sts     (g_adcAveragerVoltageAcc + 1), R21
    lds     R21, (g_adcAveragerVoltageAcc + 2)
    adc     R21, R20
    sts     (g_adcAveragerVoltageAcc + 2), R21

    lds     R21, (g_adcAveragerCurrentAcc + 0)
    add     R21, R16
    sts     (g_adcAveragerCurrentAcc + 0), R21
    lds     R21, (g_adcAveragerCurrentAcc + 1)
    adc     R21, R17
    sts     (g_adcAveragerCurrentAcc + 1), R21
    lds     R21, (g_adcAveragerCurrentAcc + 2)
    adc     R21, R20
    sts     (g_adcAveragerCurrentAcc + 2), R21

    ; Reset the ADC accumulators here and enable interrupts so
    ; the code below may run longer and the next timer interrupt will
    ; work as designed
    sts     (g_adcVoltageAcc + 0), R20
    sts     (g_adcVoltageAcc + 1), R20
    sts     (g_adcCurrentAcc + 0), R20
    sts     (g_adcCurrentAcc + 1), R20
    sei

    ; Check if PID is switched off
    lds     R20, (g_pidMode)
    cpi     R20, PID_MODE_OFF
    breq    tm0_pid_off

    ; *** PID ***
    ; PID algorithm starts here
    ; Calculate the difference between the desired and actual current values
    lds     R20, (g_pidTargetCurrent + 0)
    lds     R21, (g_pidTargetCurrent + 1)
    sub     R20, R16
    sbc     R21, R17
    ; R21:R20 = CDiff = g_pidTargetCurrent - g_adcCurrent = [-4092, 4092]

    ; If the actual current is greater than desired, switch to CC mode
    brmi    pid_switch_to_cc

    ; Calculate the difference between the desired and actual voltage values
    lds     R16, (g_pidTargetVoltage + 0)
    lds     R17, (g_pidTargetVoltage + 1)
    sub     R16, R18
    sbc     R17, R19
    ; R17:R16 = VDiff = g_pidTargetVoltage - g_adcVoltage = [-4092, 4092]

    ; If the actual voltage is greater than desired, switch to CV mode
    brmi    pid_switch_to_cv

    ; Both voltage and current are below set limits.
    ; Continue with the currently active mode.
    lds     R18, (g_pidMode)
    cpi     R18, PID_MODE_CC
    breq    pid_update_integral

    ; If not CC, than it's CV...

pid_switch_to_cv:
    ; Switch mode to CV (it won't hurt if we're alreay in CV)
    ldi     R18, PID_MODE_CV
    sts     (g_pidMode), R18

    ; Put VDiff to R21:R20 and multiply it by 4 so the CV PID is
    ; a little more aggresive than the CC one
    movw    R20, R16
    lsl     R20
    rol     R21
    lsl     R20
    rol     R21
    ; R21:R20 = VDiff*4

    rjmp    pid_update_integral

pid_switch_to_cc:
    ; Switch mode to CC
    ldi     R18, PID_MODE_CC
    sts     (g_pidMode), R18

pid_update_integral:
    ; R21:R20 = diff (either VDiff*4 or CDiff), 
    lds     R16, (g_pidIntegral + 0)
    lds     R17, (g_pidIntegral + 1)
    lds     R18, (g_pidIntegral + 2)

    clr     R19
    sbrc    R21, 7
    dec     R19
    ; R19:R21:R20 = diff

    add     R16, R20
    adc     R17, R21
    adc     R18, R19
    ; R18:R17:R16 = g_pidIntegral + diff

    ; Limit the PID integral value to prevent long recovery from a border state.
    ; Normally PID integral value must be in [0, 0x020000]
    brpl    pid_no_underflow

    clr     R16
    clr     R17
    clr     R18
    rjmp    pid_no_overflow

pwm_no_underflow:
    cpi     R18, 1
    brcc    pwm_overflow
    cpi     R17, 0xFF
    brcs    pwm_set

pwm_overflow:
    clr     R16
    ldi     R17, 0xFF
    rjmp    pwm_set

pid_no_underflow:
    cpi     R18, 0x02
    brcs    pid_no_overflow

    clr     R16
    clr     R17
    ldi     R18, 0x02

pid_no_overflow:
    sts     (g_pidIntegral + 0), R16
    sts     (g_pidIntegral + 1), R17
    sts     (g_pidIntegral + 2), R18

    ; PWM = diff/2 + g_pidIntegral/2
    
    asr     R19
    ror     R21
    ror     R20
    ; R19:R21:R20 = diff/2

    asr     R18
    ror     R17
    ror     R16
    ; R18:R17:R16 = g_pidIntegral/2

    add     R16, R20
    adc     R17, R21
    adc     R18, R19
    ; R18:R17:R16 = PWM

    ; Limit PWM value to [0, 0xFF00]
    brpl    pwm_no_underflow

pwm_underflow:    
    clr     R16
    clr     R17

pwm_set:
    cli
    sts     (g_pwmValue + 0), R16
    sts     (g_pwmValue + 1), R17
    sei

    ; *** Encoder ***
    in      R16, (PIND)
    andi    R16, BV(PD_ENCODER_DATA) | BV(PD_ENCODER_CLOCK)
    lds     R17, (g_encoderState)
    eor     R17, R16
    breq    encoder_not_changed

    sts     (g_encoderState), R16

    ; Check encoder clock, detect the rising edge (0 -> 1 transition)
    and     R17, R16
    sbrs    R17, PD_ENCODER_CLOCK
    rjmp    encoder_not_changed

    lds     R17, (g_encoderCounter)
    inc     R17
    sbrc    R16, PD_ENCODER_DATA
    subi    R17, 2
    sts     (g_encoderCounter), R17

    lds     R17, (g_settings + OFFSET_SETTINGS_BEEP_LENGTH)
    sts     (g_keyBeepLengthLeft), R17

encoder_not_changed:
    ; Check if we've accumulated 256 samples
    lds     R16, (g_adcAveragerCounter)
    inc     R16
    sts     (g_adcAveragerCounter), R16

    breq    .+2
    rjmp    tm0_notResetAverager

    ; Copy voltage values (divide by 256)
    lds     R16, (g_adcAveragerVoltageAcc + 1)
    lds     R17, (g_adcAveragerVoltageAcc + 2)
    sts     (g_adcVoltageAverage + 0), R16
    sts     (g_adcVoltageAverage + 1), R17

    ; Copy current values (divide by 256)
    lds     R16, (g_adcAveragerCurrentAcc + 0)
    lds     R17, (g_adcAveragerCurrentAcc + 1)
    lds     R18, (g_adcAveragerCurrentAcc + 2)
    sts     (g_adcCurrentAverage + 0), R17
    sts     (g_adcCurrentAverage + 1), R18

    ; Reset averager accumulators
    clr     R21
    sts     (g_adcAveragerVoltageAcc + 0), R21
    sts     (g_adcAveragerVoltageAcc + 1), R21
    sts     (g_adcAveragerVoltageAcc + 2), R21

    sts     (g_adcAveragerCurrentAcc + 0), R21
    sts     (g_adcAveragerCurrentAcc + 1), R21
    sts     (g_adcAveragerCurrentAcc + 2), R21

    ; How do we calculate the delivered capacity?
    ; Normally, for a constant current the capacity delivered in t seconds is calculated as:
    ;
    ; C = I*t/3600 [Ah]
    ;
    ; If the current is not constant, then we need to integrate its value over time.
    ; We also don't need to multiply the result by t since integral will do this for us:
    ;
    ; C = Int(I(t)*dt)/3600
    ;
    ; In case of descrete calculations, the integral can be substituted with a sum:
    ;
    ; C = Sum(I0 ... In-1)*t/(3600*n)
    ;
    ; If we sample current at a constant frequency F, n = t*F, and we can rewrite the formula as:
    ;
    ; C = Sum(I)*t/(3600*t*F) = Sum(I)/(3600*F)
    ;
    ; As you see, we don't need time t anymore! We just need to take the sum and divide it
    ; by (3600*F) which is a constant. Cool.
    ;
    ; Now let's take a step back and remember how we calculate the real user-friendly
    ; current value and put it to the previous capacity formula because we need the
    ; user-friendly displayable capacity value:
    ;
    ; Ix1000 = (Iadc + Ioffset)*Ki/4096
    ; Cx1000 = Sum((Iadc + Ioffset)*Ki)/(4096*3600*F)
    ;
    ; Since Ki is a constant here:
    ;
    ; Cx1000 = Sum(Iadc + Ioffset)*Ki/(4096*3600*F)
    ;
    ; We could take Ioffset out of the sum too, but then we would need to multiply Ioffset by the
    ; number of Sum() samples and this would require us to count these samples. But we don't
    ; want any additional big-ass counter, so we'll better sum the already offsetted current values.
    ;
    ; So, basically, that's it and the final formula is (F = 62500/8 = 7812.5):
    ;
    ; Cx1000 = Sum()*Ki/(4096*28125000)
    ;
    ; Now let's calculate operand sizes for the each step of computations.
    ;
    ; Maximum current value is 1023*4 = 4092. We sum them 7812.5 times a second, so the maximum value
    ; is 31968750 per second, 115087500000 per hour and 11508750000000 for 100 hours (the maximum sane
    ; charge time). And we need log2(N) = 43.4 bits to store it. Let's round it up to 48 bits or
    ; 6 bytes.
    ;
    ; Obviously it's a way too hard for an 8-bit MCU to multiply and divide 48-bit values. Also
    ; we don't need such a huge precision to print the delivered capacity, it's totally OK to
    ; print it as XX.XXX Ah or XXX.XX Ah for larger values. So, basically, the final number
    ; may have at least 16-bit resolution.
    ;
    ; Good, then let's firstly divide the sum by 64K, since it's completely free:
    ;
    ; Cx1000 = (Sum()/64K)*Ki/1757812.5 ~= (Sum()/64K)*Ki/1757813
    ;
    ; Looks better, but the max Sum() will be 175609588 and Ki is about 12000, so their product
    ; is about 2107315056000 and does not fit in 32 bits (it requires 41 bits). And we can't just
    ; divide Sum() by 512 more since now we're going to lose precision.
    ;
    ; The solution here will be to additionally divide Sum() by 64:
    ;
    ; Cx1000 ~= (Sum()/4M)*Ki/27466
    ;
    ; This will work OK and fit in 32 bits for Sum()/4M up to about 250K: 250K*12K = 3B,
    ; and this is enough to get 3B/27466 ~= 109 Ah, which is even more than we can print in the
    ; XX.XXX form.
    ;
    ; But the maximum possible Sum()/4M is 2743899, which is still 11 times greater. Bummer? Nope.
    ; We can additionally divide it by 10 in this case, since we are going to print it as XXX.XX!
    ; So, the final algorithm will be:
    ;
    ; 1. Calculate Sum()/4M by loading higher bytes of Sum() and shifting them two times left.
    ; 2. Check if the Sum()/4M is greater than 250K. If it is, divide it by 10 and set the XXX.XX format flag.
    ; 3. Multiply Sum by Ki and divide the result by 27466, these operations fits in 32 bit.
    ; 4. If the result is greater than 65535, divide it by 10 and set the XXX.XX format flag.
    ; 5. Print the result in the XX.XXX or XXX.XX format using the standard 16-bit print routine.
    ; 6. ???
    ; 7. PROFIT

    ; Let's get back to the code. We have ADCcurrent*256 in R18:R17:R16
    lds     R19, (g_settings + OFFSET_SETTINGS_CURRENT_OFFSET)
    add     R17, R19
    adc     R18, R21
    sbrc    R19, 7
    dec     R18

    ; Don't allow the current sum to be negative
    brpl    current_sum_positive

    clr     R16
    clr     R17
    clr     R18

current_sum_positive:
    lds     R19, (g_totalCurrentSum + 0)
    add     R19, R16
    sts     (g_totalCurrentSum + 0), R19

    lds     R19, (g_totalCurrentSum + 1)
    adc     R19, R17
    sts     (g_totalCurrentSum + 1), R19

    lds     R19, (g_totalCurrentSum + 2)
    adc     R19, R18
    sts     (g_totalCurrentSum + 2), R19

    lds     R19, (g_totalCurrentSum + 3)
    adc     R19, R21
    sts     (g_totalCurrentSum + 3), R19

    lds     R19, (g_totalCurrentSum + 4)
    adc     R19, R21
    sts     (g_totalCurrentSum + 4), R19

    lds     R19, (g_totalCurrentSum + 5)
    adc     R19, R21
    sts     (g_totalCurrentSum + 5), R19

tm0_notResetAverager:

    ; *** 100 Hz timer ***
    lds     R16, (g_timer625DividerCounter + 0)
    lds     R17, (g_timer625DividerCounter + 1)
    subi    R16, lo8(-8)
    sbci    R17, hi8(-8)
    brmi    not_100Hz_timer
    
    subi    R16, lo8(625)
    sbci    R17, hi8(625)
    sts     (g_timer625DividerCounter + 0), R16
    sts     (g_timer625DividerCounter + 1), R17

    ; R16 - R21 are already saved
    MPUSH   0, 1
    MPUSH   22, 27
    MPUSH   30, 31

    clr     R1
    call    Timer100Hz

    MPOP    30, 31
    MPOP    22, 27
    MPOP    0, 1

    pop     R21
    pop     R20
    rjmp    tm0_ret

not_100Hz_timer:
    sts     (g_timer625DividerCounter + 0), R16
    sts     (g_timer625DividerCounter + 1), R17

    pop     R21
    pop     R20
    rjmp    tm0_ret

