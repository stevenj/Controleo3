#ifndef __SIMPLEPIO_H__
#define __SIMPLEPIO_H__

// Gives us our ASF Port Defintions this is built upon.
#include "samd21.h"

// Simple Port IO Access, using direct ASF Port Mappings
// All Macros should evalute out as constant expressions, at compile time.

// Port A Bits are numbered 0-31.
#define PA(X) (X)
// Port B Bits are numbered 32-63.
#define PB(X) (X | 0x20)

// Mask to get RAW port number.
#define PMASK (0x1F)

// Get the raw port bit number, ignoring the port group.
#define PRAW(X) (X & PMASK)

// Get the raw port group.
#define PGRP(X) ((X & 0x20) >> 5)

// Port as a bit mask. (Preserving Port)
#define PBIT(X) ((uint64_t)1 << X)

// Port as a bit mask. (Ignoring Port)
#define PBITRAW(X) (PBIT(PRAW(X)))

// Port Register Access
#define PORT_DIR(X)    (PORT->Group[PGRP(X)].DIR.reg)
#define PORT_DIRCLR(X) (PORT->Group[PGRP(X)].DIRCLR.reg)
#define PORT_DIRSET(X) (PORT->Group[PGRP(X)].DIRSET.reg)
#define PORT_DIRTGL(X) (PORT->Group[PGRP(X)].DIRTGL.reg)

#define PORT_OUT(X)    (PORT->Group[PGRP(X)].OUT.reg)
#define PORT_OUTCLR(X) (PORT->Group[PGRP(X)].OUTCLR.reg)
#define PORT_OUTSET(X) (PORT->Group[PGRP(X)].OUTSET.reg)
#define PORT_OUTTGL(X) (PORT->Group[PGRP(X)].OUTTGL.reg)

#define PORT_IN(X)     (PORT->Group[PGRP(X)].IN.reg)

#define PORT_CTRL(X)   (PORT->Group[PGRP(X)].CTRL.reg)
#define PORT_WRCFG(X)  (PORT->Group[PGRP(X)].WRCONFIG.reg)

// Note Port Mux has two bits in a single 8 bit register.
#define PORT_PMUX(X)   (PORT->Group[PGRP(X)].PMUX[PRAW(X)>>1].reg)
#define PORT_PINCFG(X) (PORT->Group[PGRP(X)].PINCFG[PRAW(X)])

// This is C Code, however if bits is constant, this should all resolve at compile time.
// Should work out as an optimal write to set the SET of inputs, to one or two registers,
// as required.
static inline void PORT_INPUT(uint64_t bits) {
    if ((bits & 0xFFFFFFFF) != 0) { PORT_DIRCLR(PA(0)) = bits & 0xFFFFFFFF; }
    bits >>= 32;
    if (bits != 0) { PORT_DIRCLR(PB(0)) = bits; }
}

// This is C Code, however if bits is constant, this should all resolve at compile time.
// Should work out as an optimal write to set the SET of outputs, to one or two registers,
// as required.
static inline void PORT_OUTPUT(uint64_t bits) {
    if ((bits & 0xFFFFFFFF) != 0) { PORT_DIRSET(PA(0)) = bits & 0xFFFFFFFF; }
    bits >>= 32;
    if (bits != 0) { PORT_DIRSET(PB(0)) = bits; }
}

// This is C Code, however if bits is constant, this should all resolve at compile time.
// Should work out as an optimal write to clear the outputs, to one or two registers,
// as required.
static inline void PORT_OUTPUT_LOW(uint64_t bits) {
    if ((bits & 0xFFFFFFFF) != 0) { PORT_OUTCLR(PA(0)) = bits & 0xFFFFFFFF; }
    bits >>= 32;
    if (bits != 0) { PORT_OUTCLR(PB(0)) = bits; }
}

// This is C Code, however if bits is constant, this should all resolve at compile time.
// Should work out as an optimal write to set the outputs, to one or two registers,
// as required.
static inline void PORT_OUTPUT_HIGH(uint64_t bits) {
    if ((bits & 0xFFFFFFFF) != 0) { PORT_OUTSET(PA(0)) = bits & 0xFFFFFFFF; }
    bits >>= 32;
    if (bits != 0) { PORT_OUTSET(PB(0)) = bits; }
}

#endif
