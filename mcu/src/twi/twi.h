#pragma once

// Two-wire interface

#include "../data.h"

#define TWI_ADDR_TMP100BOARD 0b01001000

namespace twi {

void Init();

bool IsBusy();
bool SendBytes(uint8_t deviceAddress, const uint8_t* buffer, uint8_t count);
bool RecvBytes(uint8_t deviceAddress, uint8_t* buffer, uint8_t count);

extern "C" {

var volatile uint8_t g_twiState;
var uint8_t g_twiTargetAddress;
var uint8_t g_twiBuffer[8];
var volatile uint8_t* g_twiDataAddress;
var volatile uint8_t g_bytesCount;

} // extern "C"

} // namespace twi
