
; Assembler programming rules:
; 1. Registers that must be saved by a function:
;    R2-R17, R28-R29, R1 is and must be zero
;
; 2. Registers that can be used without saving:
;    R0, R18-R27, R30-R31
;
; 3. Parameters sizes are rounded up to the next even number (i.e., 2, 4, 6, 8)
;    and they go downwards starting from R25. For example:
;    int func(char a, long b);
;    a -> R24, b -> R23:R22:R21:R20, result -> R25:R24

; Define register offset here as 0 to be able to use the in/out commands
#define __SFR_OFFSET 0
#include "common.h"

; Multiple push: pushes registers starting from 'first' and
; ending with 'last' to the stack
.altmacro
.macro MPUSH first, last
    push    R\first
.if \first != \last
    MPUSH   %(\first + 1), \last
.endif
.endm

; Multiple pop, does exactly the opposite of MPUSH (in the correct reverse order!)
.altmacro
.macro MPOP first, last
.if \first != \last
    MPOP    %(\first + 1), \last
.endif
    pop     R\first
.endm

