#include "includes.h"

namespace utils {

void DoSoftReset()
{
    cli();

    // Disable PWM
    TCCR0A = 0;

    // Switch off output relay
    PORTD &= ~BV(PD_RELAY);

    // Make some "reset" sound    
    ICR1H = HIBYTE(1500);
    ICR1L = LOBYTE(1500);
    OCR1AH = 0;
    OCR1AL = 64;
    display::HardDelay(30);

    ICR1H = HIBYTE(3000);
    ICR1L = LOBYTE(3000);
    display::HardDelay(30);

    OCR1AH = 0;
    OCR1AL = 0;

    asm volatile ("jmp 0");
}

void InitMcu()
{
    // *** ADC ***
    // Set ADC clock to 1 MHz, thus a single conversion will take 208 CPU clocks
    DIDR0 = BV(ADC0D) | BV(ADC1D);
    ADMUX = 0u;
    ADCSRA = BV(ADEN) | BV(ADPS2);

    // Start the first conversion and wait until it completes
    // (since it takes 25 ADC clocks instead of 13)
    ADCSRA = BV(ADEN) | BV(ADSC) | BV(ADPS2);
    while (ADCSRA & BV(ADSC))
    {}

    // Set encoder clock to high to prevent accidental rising edge detection
    g_encoderState = BV(PD_ENCODER_CLOCK);

    // *** Timer0 ***
    // Timer0: PWM and fan
    // Clear OC0A on Compare Match, set OC0A at BOTTOM (COM0A = 10),
    // FastPWM [0 -> 0xFF] (WGM0 = 011)
    OCR0A = OCR0B = 0;
    TCCR0A = BV(COM0B1) | BV(WGM01) | BV(WGM00);

    // Enable overflow interrupt
    TIMSK0 = BV(TOIE0);

    // Start timer at 16 MHz (CS0 = 001)
    TCCR0B = BV(CS00);

	// *** Timer1 ***
    // Timer1: Sound generator
	// Set TOP as 1000
    ICR1H = (1000 >> 8);
    ICR1L = (1000 & 0xFF);

	// Set PWM level to 0 to mute sound
    OCR1AH = 0;
    OCR1AL = 0;

	// Enable OC1A, PWM, Phase and Frequency Correct,
	// TOP at ICR1, Clock = CLKio/8 = 2 MHz, start timer
    TCCR1A = BV(COM1A1);
    TCCR1B = BV(WGM13) | BV(CS11);

    // *** Pins ***
    DDRB = BV(PB_DISPLAY_DC) | BV(PB_OC1A) | BV(PB_DISPLAY_RES) | BV(PB_DISPLAY_SDA) | BV(PB_DISPLAY_SCL);
    DDRD = BV(PD_RELAY) | BV(PD_OC0B) | BV(PD_OC0A);

    // *** SPI ***
    // SPI: Display. SPI must be initialized after setting pin direction
    SPSR = BV(SPI2X);
    SPCR = BV(SPE) | BV(MSTR) | BV(CPOL) | BV(CPHA);

    twi::Init();

    sei();
}

void Delay(uint8_t n10msTicks)
{
    uint8_t lastCycle = g_100HzCounter + n10msTicks;
    while (g_100HzCounter != lastCycle)
        asm volatile ("sleep");
}

void VoltageToString(uint16_t x1000Voltage, bool bLeadingSpace)
{
    x1000Voltage += 5;
    I16ToString(x1000Voltage, g_buffer, bLeadingSpace ? 1 : 0);
    g_buffer[4] = g_buffer[3];
    g_buffer[3] = g_buffer[2];
    g_buffer[2] = '.';
    g_buffer[5] = 'V';
}

void CurrentToString(uint16_t x1000Current)
{
    x1000Current += 5;
    if (x1000Current > 9999)
        x1000Current = 9999;

    I16ToString(x1000Current, g_buffer, 1);
    g_buffer[0] = g_buffer[1];
    g_buffer[1] = '.';
    g_buffer[4] = 'A';
}

void WattageToString(uint16_t x1000Voltage, uint16_t x1000Current)
{
    uint16_t x100Wattage = static_cast<uint16_t>(static_cast<uint32_t>(x1000Voltage)*x1000Current/10000) + 5;
    I16ToString(x100Wattage, g_buffer, 2);
    g_buffer[4] = g_buffer[3];
    g_buffer[3] = '.';
    g_buffer[5] = 'W';
}

void CapacityToString()
{
    bool largeCapacity = false;
    uint32_t currentSum = GetCurrentSumDiv4M();
    if (currentSum > 250000ul)
    {
        currentSum /= 10;
        largeCapacity = true;
    }

    currentSum *= g_settings.m_current4096Value;
    currentSum /= 27466;
    if (currentSum > 65535ul)
    {
        currentSum /= 10;
        largeCapacity = true;
    }

    I16ToString(static_cast<uint16_t>(currentSum), g_buffer, 0);
    g_buffer[5] = g_buffer[4];
    g_buffer[4] = g_buffer[3];
    if (largeCapacity)
    {
        g_buffer[3] = '.';
    }
    else
    {
        g_buffer[3] = g_buffer[2];
        g_buffer[2] = '.';
    }

    if (g_buffer[0] == '0')
        g_buffer[0] = 127;

    g_buffer[6] = 'A';
    g_buffer[7] = 'h';
}

void TimeToString()
{
    I8ToString(g_time[1], g_buffer + 5);
    I8ToString(g_time[2], g_buffer + 2);
    I8ToString(g_time[3], g_buffer + 8);
    g_buffer[0] = g_buffer[9];
    g_buffer[1] = g_buffer[10];
    g_buffer[2] = g_buffer[5] = ':';
}

void TimeCapacityReset()
{
    cli();
    g_totalCurrentSum[0] = g_totalCurrentSum[1] = g_totalCurrentSum[2] = 0;
    g_totalCurrentSum[3] = g_totalCurrentSum[4] = g_totalCurrentSum[5] = 0;
    g_time[0] = g_time[1] = g_time[2] = g_time[3] = 0;
    sei();
}

void TemperatureToString(int16_t x100Temp)
{
    x100Temp += 5;

    // We don't work with negative values
    if (x100Temp < 0)
        x100Temp = 0;
    
    // Maximum possible value is 99.99 degrees
    else if (x100Temp > 9999)
        x100Temp = 9999;

    I16ToString(static_cast<uint16_t>(x100Temp), g_buffer, 1);
    g_buffer[4] = g_buffer[3];
    g_buffer[3] = '.';
    g_buffer[5] = 0x80;
}

uint16_t GetBoardTempColor(uint16_t temperature)
{
    uint8_t t = HIBYTE(temperature);
    if (t < 58)
        return CLR_GREEN;

    if (t < 74)
        return RGB((t - 58) << 4, 255, 0);

    if (t < 90)
        return RGB(255, (89 - t) << 4, 0);

    return CLR_RED;
}

void PercentToString(uint8_t percent)
{
    I8ToStringSpaces(percent);
    g_buffer[3] = '%';
}

void I8ToStringSpaces(uint8_t value)
{
    I8ToString(value, g_buffer);
    if (g_buffer[0] == '0')
    {
        g_buffer[0] = 127;
        if (g_buffer[1] == '0')
            g_buffer[1] = 127;
    }
}

void I8SToString(int8_t value)
{
    if (value >= 0)
    {
        //I8ToString(
    }
    else
    {
    }
}


uint16_t ChangeI16ByDigit(uint16_t value, uint8_t digit, int8_t delta, uint16_t minValue, uint16_t maxValue)
{
    uint16_t change = 1;
    if (digit == 1)
        change = 10;
    else if (digit == 2)
        change = 100;
    else if (digit == 3)
        change = 1000;
    else if (digit == 4)
        change = 10000;

    if (delta > 0)
    {
        change *= static_cast<uint8_t>(delta);
        if (0xFFFF - value < change)
            return maxValue;

        value += change;
        return value > maxValue ? maxValue : value;
    }

    change *= static_cast<uint8_t>(-delta);
    if (change > value)
        return minValue;

    value -= change;
    return value < minValue ? minValue : value;
}

uint8_t ChangeI8ByDelta(uint8_t value, int8_t delta, int8_t minValue, int8_t maxValue)
{
    if (delta > 0)
    {
        if (0xFF - value < static_cast<uint8_t>(delta))
            return maxValue;

        value += delta;
        return value > maxValue ? maxValue : value;
    }

    if (static_cast<uint8_t>(-delta) > value)
        return minValue;

    value += delta;
    return value < minValue ? minValue : value;
}

void ClearPendingKeys()
{
    g_encoderKey = EEncoderKey::None;
    g_encoderCounter = 0;
}

} // namespace utils

