#ifndef __SIMPLEPIO_H__
#define __SIMPLEPIO_H__

// Gives us our ASF Port Defintions this is built upon.
#include "samd21.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
#define PBITRAW(X) (1 << PRAW(X))

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

#define PINCFG_INPUT      (PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_INEN)
#define PINCFG_INPUT_PULL (PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_PULLEN | PORT_WRCONFIG_INEN)
#define PINCFG_OUTPUT     (PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_INEN)
#define PINCFG_OUTPUT_HD  (PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_DRVSTR | PORT_WRCONFIG_INEN)
#define PINCFG_PERIPHERAL(x) (PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(x) | PORT_WRCONFIG_INEN | PORT_WRCONFIG_PMUXEN)

#ifndef __ASSEMBLY__
// This is C Code, however if bits is constant, this should all resolve at compile time.
// Should work out as an optimal write to set the SET of inputs, to one or two registers,
// as required.
static inline void PORT_INPUT(const uint64_t bits) {
    if ((bits & 0xFFFFFFFF)         != 0) { PORT_DIRCLR(PA(0)) = bits & 0xFFFFFFFF; }
    if ((bits & 0xFFFFFFFF00000000) != 0) { PORT_DIRCLR(PB(0)) = bits >> 32; }
}

// This is C Code, however if bits is constant, this should all resolve at compile time.
// Should work out as an optimal write to set the SET of outputs, to one or two registers,
// as required.
static inline void PORT_OUTPUT(const uint64_t bits) {
    if ((bits & 0xFFFFFFFF)         != 0) { PORT_DIRSET(PA(0)) = bits & 0xFFFFFFFF; }
    if ((bits & 0xFFFFFFFF00000000) != 0) { PORT_DIRSET(PB(0)) = bits >> 32; }
}

// This is C Code, however if bits is constant, this should all resolve at compile time.
// Should work out as an optimal write to clear the outputs, to one or two registers,
// as required.
static inline void PORT_OUTPUT_LOW(const uint64_t bits) {
    if ((bits & 0xFFFFFFFF)         != 0) { PORT_OUTCLR(PA(0)) = bits & 0xFFFFFFFF; }
    if ((bits & 0xFFFFFFFF00000000) != 0) { PORT_OUTCLR(PB(0)) = bits >> 32; }
}

// This is C Code, however if bits is constant, this should all resolve at compile time.
// Should work out as an optimal write to set the outputs, to one or two registers,
// as required.
static inline void PORT_OUTPUT_HIGH(const uint64_t bits) {
    if ((bits & 0xFFFFFFFF)         != 0) { PORT_OUTSET(PA(0)) = bits & 0xFFFFFFFF; }
    if ((bits & 0xFFFFFFFF00000000) != 0) { PORT_OUTSET(PB(0)) = bits >> 32; }
}

// Write the same configuration to MULTIPLE Pins.
// Should optimise down to the required number of
// constant writes.  Should be no worse than individual
// pin configuration, and potentially, A LOT better.
static inline void PORT_CONFIGURE(const uint64_t bits, const uint32_t cfg) {
    if ((bits & 0x000000000000FFFF) != 0) { 
        PORT_WRCFG(PA(0)) = cfg | (bits & 0xFFFF);
    }
    if ((bits & 0x00000000FFFF0000) != 0) { 
        PORT_WRCFG(PA(16)) = cfg | PORT_WRCONFIG_HWSEL | ((bits >> 16) & 0xFFFF);
    }
    if ((bits & 0x0000FFFF00000000) != 0) { 
        PORT_WRCFG(PB(0)) = cfg | ((bits >> 32) & 0xFFFF);
    }
    if ((bits & 0xFFFF000000000000) != 0) { 
        PORT_WRCFG(PB(16)) = cfg | PORT_WRCONFIG_HWSEL | ((bits >> 48) & 0xFFFF);
    }
}

static inline void PORT_SET_PMUX(const uint64_t port, const uint8_t mux) {
    PORT_PMUX_Type pmux = {0};

    pmux.reg = PORT_PMUX(port);
    if ((port & 1) == 0) {
        pmux.bit.PMUXE = mux;
    } else {
        pmux.bit.PMUXO = mux;
    }
    PORT_PMUX(port) = pmux.reg;
}
#endif

// SERCOM HELPER MACROS
#define SERCOM_APBCMASK(x) SERCOM_APBCMASK_(x)
#define SERCOM_APBCMASK_(x) (PM->APBCMASK.bit.SERCOM ## x ## _)

#define SERCOM_GCLKID(x) SERCOM_GCLKID_(x)
#define SERCOM_GCLKID_(x) (GCLK_CLKCTRL_ID_SERCOM ## x ## _CORE_Val)

#define SERCOM(x) SERCOM_(x)
#define SERCOM_(x) (SERCOM ## x )

// TCC HELPER MACROS
#define TCC(x) TCC_(x)
#define TCC_(x) TCC ## x

#define TCC_APBCMASK(x) TCC_APBCMASK_(x)
#define TCC_APBCMASK_(x) (PM->APBCMASK.bit.TCC ## x ## _)


// External Interrupt convenience macros
#define EXTINT(x)  x

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
