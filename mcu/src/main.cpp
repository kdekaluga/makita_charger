#include "includes.h"

void CheckForFailures()
{
    uint8_t failureState = g_failureState;

    // Low power check.
    // Rule: if power is continuously low for 500 ms, switch off the device.
    // If power is good for at least 100 ms, switch device back on.
    static int8_t lowPowerCounter = 0;
    if (PINC & BV(PC_IN_POWER_OK))
    {
        lowPowerCounter += 5;
        if (lowPowerCounter >= 50)
        {
            lowPowerCounter = 50;
            failureState &= ~FAILURE_POWER_LOW;
        }
    }
    else
    {
        if (--lowPowerCounter < 0)
        {
            lowPowerCounter = 0;
            failureState |= FAILURE_POWER_LOW;
        }
    }

    cli();
    uint16_t voltage = g_adcVoltageAverage;
    uint16_t current = g_adcCurrentAverage;
    sei();

    // Overvoltage check.
    // Rule: if output voltage exceeds the set value by more than 1 V and output current exceeds
    // the set value by more than 150 mA for 250 ms, the overvoltage failure is signaled.
    static int8_t overvoltageCounter = 0;
    if (static_cast<int16_t>(voltage - g_pidTargetVoltage) > 170 &&
        static_cast<int16_t>(current - g_pidTargetCurrent) > 50)
    {
        if (++overvoltageCounter >= 25)
            failureState |= FAILURE_OVERVOLTAGE;
    }
    else
    {
        overvoltageCounter = 0;
    }

    // Overcurrent check.
    // Rule: if output current exceeds 9A for 150 ms, the overcurrent failure is signaled.
    static int8_t overcurrentCounter = 0;
    if (current > 3072)
    {
        if (++overcurrentCounter >= 15)
            failureState |= FAILURE_OVERCURRENT;
    }
    else
    {
        overcurrentCounter = 0;
    }

    if (failureState & FAILURE_ANY)
    {
        // Switch off output relay and PWM generator
        PORTD &= ~BV(PD_RELAY);
        g_pidMode = PID_MODE_OFF;
        
        // Clear FAILURE_NONE and failure counter
        g_failureState = failureState & FAILURE_ANY;
        return;
    }

    // We're good, allow PWM to run
    if (failureState & FAILURE_NONE)
    {
        if (g_outOn)
        {
            if (g_pidMode == PID_MODE_OFF)
                g_pidMode = PID_MODE_CC;
        }
        else
        {
            g_pidMode = PID_MODE_OFF;
        }

        return;
    }

    // Enable relay and wait 20 msec, then set the FAILURE_NONE flag.
    // PWM will be switched on in the next CheckForFailures() call.
    PORTD |= BV(PD_RELAY);
    if (++failureState >= 3)
        failureState = FAILURE_NONE;

    g_failureState = failureState;
}

void ProcessEncoderButton()
{
    static uint16_t buttonDownTime = 0;
    if (!(PIND & BV(PD_ENCODER_SWITCH)))
    {
        if (++buttonDownTime == 2)
        {
            g_encoderKey = EEncoderKey::Down;
            sound::Beep();
        }
        else if (buttonDownTime == 100)
        {
            g_encoderKey = EEncoderKey::DownLong;
            sound::Beep();
        }
        else if (buttonDownTime == 600)
        {
            utils::DoSoftReset();
        }
    }
    else
    {
        if (buttonDownTime >= 100)
            g_encoderKey = EEncoderKey::UpLong;

        else if (buttonDownTime >= 2)
            g_encoderKey = EEncoderKey::Up;

        buttonDownTime = 0;
    }
}

void RequestTemperature()
{
    if (twi::IsBusy())
        return;

    switch (g_tempRequesterState++)
    {
    case 0:
        // Read temperature (result from the previous request), exchange bytes
        *reinterpret_cast<uint8_t*>(&g_temperatureBoard) = *reinterpret_cast<uint8_t*>(twi::g_twiBuffer + 1);
        *(reinterpret_cast<uint8_t*>(&g_temperatureBoard) + 1) = *reinterpret_cast<uint8_t*>(twi::g_twiBuffer);

        // Set resolution to 12 bit
        twi::g_twiBuffer[0] = 0x01;
        twi::g_twiBuffer[1] = 0x60;
        twi::SendBytes(TWI_ADDR_TMP100BOARD, twi::g_twiBuffer, 2);
        break;

    case 1:
        // Select the temperature register
        twi::g_twiBuffer[0] = 0x00;
        twi::SendBytes(TWI_ADDR_TMP100BOARD, twi::g_twiBuffer, 1);
        break;

    case 2:
        // Request the temperature
        twi::g_twiBuffer[0] = 99;
        twi::g_twiBuffer[1] = 0;
        twi::RecvBytes(TWI_ADDR_TMP100BOARD, twi::g_twiBuffer, 2);

    default:
        g_tempRequesterState = 0;
    }
}

void Timer100Hz()
{
    // Increments timer and time counters
    ++g_100HzCounter;
    if (++g_time[0] == 100)
    {
        g_time[0] = 0;
        if (++g_time[1] == 60)
        {
            g_time[1] = 0;
            if (++g_time[2] == 60)
            {
                g_time[2] = 0;
                ++g_time[3];
            }
        }
    }

    CheckForFailures();
    ProcessEncoderButton();
    sound::OnTimer();
    RequestTemperature();
}

static const char pm_mainMenuTitle[] PROGMEM = "Select mode:";
static const char pm_mainMenu0[] PROGMEM = "Charger";
static const char pm_mainMenu1[] PROGMEM = "Power Supply";
static const char pm_mainMenu2[] PROGMEM = "Settings";
static const char pm_mainMenu3[] PROGMEM = "Calibration";
static const char pm_mainMenu4[] PROGMEM = "Music player";
static const char pm_mainMenu5[] PROGMEM = "About";

static const display::Menu pm_mainMenu PROGMEM =
{
    nullptr, nullptr, nullptr,
    6,
    pm_mainMenuTitle,
    pm_mainMenu0, pm_mainMenu1, pm_mainMenu2, pm_mainMenu3, pm_mainMenu4, pm_mainMenu5
};

static const char pm_aboutTitle[] PROGMEM = "About";
static const char pm_about[] PROGMEM = "Smart Makita\ncharger by SD\n"
    "(kde@kaluga.ru)\n" VERSION_STRING ", built on:\n" __DATE__;

int main()
{
    // Switch everything to input just in case
    DDRB = DDRC = DDRD = 0;
    PORTB = PORTC = PORTD = 0;

    screen::calibration::g_voltageX1000 = 23500;
    screen::calibration::g_currentX1000 = 6000;

    static const char pm_profileName[] PROGMEM = "Test 4.2 V profile"; //"Test 4.2 V profile";
    memcpy_PF(screen::charger::g_profile.m_name, reinterpret_cast<uint_farptr_t>(pm_profileName), 18);
    screen::charger::g_profile.m_nameLength = 18;
    screen::charger::g_profile.m_chargeVoltageX1000 = 4200;
    screen::charger::g_profile.m_chargeCurrentX1000 = 500;
    screen::charger::g_profile.m_openVoltageX1000 = 4700;
    screen::charger::g_profile.m_openCurrentX1000 = 30;
    screen::charger::g_profile.m_minBatteryVoltageX1000 = 3000;
    screen::charger::g_profile.m_restartChargeVoltageX1000 = 4100;
    screen::charger::g_profile.m_use3rdPin = false;
    screen::charger::g_profile.m_stopChargeCurrentPercent = 10;

    utils::InitMcu();
    display::Init();

    if (!g_settings.ReadFromEeprom())
    {
        static const char pm_invalidSettings[] PROGMEM =
            "No valid settings\nwere found in the\nEEPROM. Settings\nare reset to\ntheir defaults.";

        g_settings.ResetToDefault();
        g_settings.SaveToEeprom();
        display::Clear(CLR_BLACK);
        display::MessageBox(display::pm_warning, pm_invalidSettings, MB_WARNING | MB_OK);
    }

    // Initially start in the charger mode
    uint8_t mode = 0;
    for (;;)
    {
        switch (mode)
        {
        case 0:
            screen::charger::Show();
            break;

        case 1:
            screen::psupply::Show();
            break;

        case 3:
            screen::calibration::Show();
            break;

        case 4:
            screen::music::Show();
            break;

        case 5:
            display::MessageBox(pm_aboutTitle, pm_about, MB_OK | MB_INFO);
            break;
        }

        mode = pm_mainMenu.Show(mode);
    }    
}
