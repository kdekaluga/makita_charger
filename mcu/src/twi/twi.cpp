#include "../includes.h"

namespace twi {

void Init()
{
    TWCR = 0;

    // Set TWI frequency to 400 KHz (prescaler = 1, bitrate = 12)
    TWSR = 0;
    TWBR = 12;

    TWCR = BV(TWINT) | BV(TWEN) | BV(TWIE);
    g_twiState = TWI_STATE_OK;
}

bool IsBusy()
{
    return g_twiState == TWI_STATE_BUSY;
}

bool SendBytes(uint8_t deviceAddress, const uint8_t* buffer, uint8_t count)
{
    if (IsBusy())
        return false;

    g_twiTargetAddress = deviceAddress << 1;
    g_twiDataAddress = const_cast<uint8_t*>(buffer);
    g_bytesCount = count;
    
    g_twiState = TWI_STATE_BUSY;
    TWCR = BV(TWINT) | BV(TWSTA) | BV(TWEN) | BV(TWIE);

    return true;
}

bool RecvBytes(uint8_t deviceAddress, uint8_t* buffer, uint8_t count)
{
    if (IsBusy())
        return false;

    g_twiTargetAddress = (deviceAddress << 1) | 0x01;
    g_twiDataAddress = buffer;
    g_bytesCount = count;

    g_twiState = TWI_STATE_BUSY;
    TWCR = BV(TWINT) | BV(TWSTA) | BV(TWEN) | BV(TWIE);

    return true;
}


} // namespace twi
