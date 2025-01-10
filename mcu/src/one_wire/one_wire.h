#pragma once

#include "../data.h"

// One wire protocol timings compatible with Makita (in 16us ticks)
#define ONE_WIRE_TIMING_RESET_LOW 47
#define ONE_WIRE_TIMING_RESET_HIGH 4
#define ONE_WIRE_TIMING_RESET_WAIT 26

#define ONE_WIRE_TIMING_OUT0_LOW 6
#define ONE_WIRE_TIMING_OUT0_HIGH 2
#define ONE_WIRE_TIMING_OUT1_LOW 1
#define ONE_WIRE_TIMING_OUT1_HIGH 7

#define ONE_WIRE_TIMING_IN_LOW 1
#define ONE_WIRE_TIMING_IN_HIGH 7

namespace one_wire {

// 1-Wire buffer for the timer ISR
var volatile uint8_t g_1WireBuffer[20];

extern "C" {

// 1-Wire counter and address for the timer ISR
var volatile uint8_t g_1WireCounter;
var volatile uint8_t* g_1WireAddress;

} // extern "C"

bool Reset();
void Send(uint8_t value);
uint8_t Recv();

} // namespace one_wire

