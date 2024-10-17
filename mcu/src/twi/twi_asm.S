#include "../assembler_defines.S"

.global TWI_vect


TWI_vect:
    push    R24
    in      R24, (SREG)
    push    R24

    ; Disable TWI interrupt requests, so we can enable interrupts right now
    ; (this is required for the correct PWM operation)
    ldi     R24, BV(TWEN)
    sts     (TWCR), R24
    sei
        
    MPUSH   30, 31
    ; Now we need not hurry...

    lds     R24, (TWSR)

    ; No relevant state
    andi    R24, 0xF8
    breq    twi_ret

    ; START sent
    cpi     R24, 0x08
    breq    send_slave_address

    ; Repeated START sent
    cpi     R24, 0x10
    breq    send_slave_address

    ; SLA+W sent, ACK received
    cpi     R24, 0x18
    breq    twi_send_next_byte

    ; SLA+W sent, NACK received
    cpi     R24, 0x20
    breq    twi_no_device

    ; Data byte sent, ACK received
    cpi     R24, 0x28
    breq    twi_send_next_byte

    ; Data byte sent, NACK received
    cpi     R24, 0x30
    breq    twi_nack_received

    ; Bus arbitration lost
    cpi     R24, 0x38
    breq    twi_bus_error

    ; SLA+R sent, ACK received
    cpi     R24, 0x40
    breq    twi_recv_first_byte

    ; SLA+R sent, NACK received
    cpi     R24, 0x48
    breq    twi_no_device

    ; Data byte received, ACK sent
    cpi     R24, 0x50
    breq    twi_byte_received

    ; We don't send NACK, so we never expect state 0x58 
    ; (data byte received, NACK sent)
    ; Any other state is also not expected.
    ldi     R24, TWI_STATE_UNKNOWN_ERROR
    rjmp    twi_set_status_stop_ret

twi_ok_stop_ret:
    ldi     R24, TWI_STATE_OK
twi_set_status_stop_ret:
    sts     (g_twiState), R24
    
    ldi     R24, BV(TWINT) | BV(TWSTO) | BV(TWEN) | BV(TWIE)
twi_set_cr_ret:
    sts     (TWCR), R24

twi_ret:
    MPOP    30, 31
    pop     R24
    out     (SREG), R24
    pop     R24
    reti

send_slave_address:
    lds     R24, (g_twiTargetAddress)
    sts     (TWDR), R24

    ldi     R24, BV(TWINT) | BV(TWEN) | BV(TWIE)
    rjmp    twi_set_cr_ret

twi_send_next_byte:
    lds     R24, (g_bytesCount)
    subi    R24, 1
    brcs    twi_ok_stop_ret
    sts     (g_bytesCount), R24

    lds     ZL, (g_twiDataAddress + 0)
    lds     ZH, (g_twiDataAddress + 1)
    ld      R24, Z+
    sts     (g_twiDataAddress + 0), ZL
    sts     (g_twiDataAddress + 1), ZH
    sts     (TWDR), R24

    ldi     R24, BV(TWINT) | BV(TWEN) | BV(TWIE)
    rjmp    twi_set_cr_ret

twi_no_device:
    ldi     R24, TWI_STATE_NO_TARGET
    rjmp    twi_set_status_stop_ret

twi_nack_received:
    lds     R24, (g_bytesCount)
    and     R24, R24
    lds     R24, TWI_STATE_ALL_SENT_NO_ACK
    breq    twi_set_status_stop_ret

    lds     R24, TWI_STATE_NO_ACK
    rjmp    twi_set_status_stop_ret

twi_bus_error:
    lds     R24, TWI_STATE_BUS_ERROR
    rjmp    twi_set_status_stop_ret

twi_recv_first_byte:
    ldi     R24, BV(TWINT) | BV(TWEA) | BV(TWEN) | BV(TWIE)
    rjmp    twi_set_cr_ret

twi_byte_received:
    lds     ZL, (g_twiDataAddress + 0)
    lds     ZH, (g_twiDataAddress + 1)
    lds     R24, (TWDR)
    st      Z+, R24
    sts     (g_twiDataAddress + 0), ZL
    sts     (g_twiDataAddress + 1), ZH

    lds     R24, (g_bytesCount)
    subi    R24, 1
    sts     (g_bytesCount), R24
    brne    twi_recv_first_byte

    rjmp    twi_ok_stop_ret
