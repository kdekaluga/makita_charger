// Common assembly/cpp include header. Defines everything needed for the assembly compiler

#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>

// Bit value
#define BV(n) (1 << n)

// *** Pins ***

// Display (port B)
#define PB_DISPLAY_RES 2
#define PB_DISPLAY_DC 0
#define PB_DISPLAY_SCL 5
#define PB_DISPLAY_SDA 3

// Encoder (port D)
#define PD_ENCODER_DATA 2
#define PD_ENCODER_CLOCK 4
#define PD_ENCODER_SWITCH 7

// Output pins
#define PD_RELAY 3
#define PB_OC1A 1
#define PD_OC0A 6
#define PD_OC0B 5

// Input pins
#define PC_IN_POWER_OK 2
#define PC_IN_BATTERY_STATUS 3

// PWM
#define ADC_CHANNEL_VOLTAGE 0
#define ADC_CHANNEL_CURRENT 1

#define ADC_SRA_VALUE (BV(ADEN) | BV(ADSC) | BV(ADPS2))

#define PID_MODE_OFF 0x00
#define PID_MODE_CV 0x01
#define PID_MODE_CC 0x02

// Display constants
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240

#define DISPLAY_CMD_SLPOUT 0x11
#define DISPLAY_CMD_NORON 0x13
#define DISPLAY_CMD_INVON 0x21
#define DISPLAY_CMD_DISPON 0x29
#define DISPLAY_CMD_CASET 0x2A
#define DISPLAY_CMD_RASET 0x2B
#define DISPLAY_CMD_RAMWR 0x2C
#define DISPLAY_CMD_COLMOD 0x3A

#define DISPLAY_DOES_NOT_FIT 255

// Quite a nasty part: avrgcc assembler cannot access C struct members since
// it does not know a member offset. So we explicitly define needed offsets here.
// And yes, if a struct definition is changed, you may need to manually fix the offsets.
#define OFFSET_SETTINGS_BEEP_LENGTH 0
#define OFFSET_SETTINGS_VOLTAGE_OFFSET 2
#define OFFSET_SETTINGS_CURRENT_OFFSET 5

// *** TWI ***

// TWI unit is currently busy and cannot accept new requests.
// TWI unit accepts new requests in any other state.
#define TWI_STATE_BUSY 0x80

// The last request was successfully executed
#define TWI_STATE_OK 0x00

// The target device did not ACKed SLA+W or SLA+R
#define TWI_STATE_NO_TARGET 0x01

// The target device did not ACKed 
#define TWI_STATE_NO_ACK 0x02

// The target device did not ACKed, but all bytes were transmitted
#define TWI_STATE_ALL_SENT_NO_ACK 0x03

// Bus arbitration error
#define TWI_STATE_BUS_ERROR 0x04

// Any other error
#define TWI_STATE_UNKNOWN_ERROR 0x05

