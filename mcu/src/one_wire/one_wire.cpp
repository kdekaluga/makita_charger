#include "../includes.h"

namespace one_wire {

#define ONE_WIRE_BUFFER(n) (g_1WireBuffer + sizeof(g_1WireBuffer) - n)
#define ONE_WIRE_BEGIN volatile uint8_t* buffer = ONE_WIRE_BUFFER(0)
#define ONE_WIRE_ADD(ticks, level) *--buffer = (ticks*2 + (level ? 1 : 0))
#define ONE_WIRE_END *--buffer = 1

void StartAndWait()
{
    g_1WireAddress = ONE_WIRE_BUFFER(0);
    g_1WireCounter = 1;

    while (g_1WireCounter)
        asm volatile ("sleep");
}

bool Reset()
{
    DDRC &= ~BV(PC_1_WIRE);

    // Check that 1-wire pin is high
    if (!(PINC & BV(PC_1_WIRE)))
    {
        utils::Delay(2);
        if (!(PINC & BV(PC_1_WIRE)))
            return false;
    }

    ONE_WIRE_BEGIN;
    ONE_WIRE_ADD(ONE_WIRE_TIMING_RESET_LOW, 0);
    ONE_WIRE_ADD(ONE_WIRE_TIMING_RESET_HIGH, 1);
    ONE_WIRE_ADD(ONE_WIRE_TIMING_RESET_WAIT, 1);
    ONE_WIRE_END;
    StartAndWait();

    return (*ONE_WIRE_BUFFER(3) & BV(PC_1_WIRE)) == 0;
}

void Send(uint8_t value)
{
    ONE_WIRE_BEGIN;
    for (uint8_t i = 1; i; i <<= 1)
    {
        if (value & i)
        {
            ONE_WIRE_ADD(ONE_WIRE_TIMING_OUT1_LOW, 0);
            ONE_WIRE_ADD(ONE_WIRE_TIMING_OUT1_HIGH, 1);
        }
        else
        {
            ONE_WIRE_ADD(ONE_WIRE_TIMING_OUT0_LOW, 0);
            ONE_WIRE_ADD(ONE_WIRE_TIMING_OUT0_HIGH, 1);
        }
    }
    
    ONE_WIRE_END;
    StartAndWait();
}

uint8_t Recv()
{
    ONE_WIRE_BEGIN;
    for (uint8_t i = 0; i < 8; ++i)
    {
        ONE_WIRE_ADD(ONE_WIRE_TIMING_IN_LOW, 0);
        ONE_WIRE_ADD(ONE_WIRE_TIMING_IN_HIGH, 1);
    }
    
    ONE_WIRE_END;
    StartAndWait();

    const volatile uint8_t* p = ONE_WIRE_BUFFER(2);
    uint8_t res = 0;
    for (uint8_t i = 1; i; i <<= 1)
    {
        if (*p & BV(PC_1_WIRE))
            res |= i;

        p -= 2;
    }
    
    return res;
}


} // namespace one_wire

