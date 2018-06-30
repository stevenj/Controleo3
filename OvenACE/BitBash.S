/*
 * Assembler Bitbashing routines for the MANY devices connected
 * to the Controlleo that are wired up in such a way as to make
 * the hardware modules in the MCU USELESS.
 *
 * EABI Quick Reference:
 * R0-R3  = Parameters, and need not be saved.
 * R4-R12 = Variables, these must be saved and restored IF used.
 * R13    = SP (Stack Pointer)
 * R14    = LR (Link Register)
 * R15    = PC (Program Counter)
 * Flags  = Condition Codes, don't need to be preserved.
 */

    .cpu    cortex-m0
    .thumb

#include "HWPinAssignments.h"

// Sends a single bit located in the carry, and negates the clock.
// 0 = 7 Cycles
// 1 = 6 Cycles
.macro sd_tx_bit
  bcs     tx_sdcard_1\@     // [2=1,1=0]
tx_sdcard_0\@:
  str     r2, [r3,  # PORT_OUTCLR_OFFSET] // Clear data bit [2]
  b       tx_sdcard_E\@      // [2]
tx_sdcard_1\@: 
  str     r2, [r3,  # PORT_OUTSET_OFFSET] // Set data bit [2]
tx_sdcard_E\@:
  str     r1, [r3,  # PORT_OUTSET_OFFSET] // Set CLK [2]
.endm
/*
 * Transmit a Byte to the SDCARD (Half Duplex, Discard Input)
 * R0 = Byte to send.
 */
.func   TX_SDCARD
.global TX_SDCARD
TX_SDCARD:
  // On Entry, CLK must be set as GPIO and must be LOW
  // On Exit CLK is LOW
  // Cycles = 4 + 1 + (6.5*8) + (3*7) + 4
  //        = 82 Cycles per Byte
  //        = ~4Mbps

  mov     r1, # PBITRAW(SDCARD_CLK)  // r1 = CLK Bitmask    [1]
  mov     r2, # PBITRAW(SDCARD_MOSI) // r2 = MOSI Bitmask   [1]
  ldr     r3, IOPortBase             // Address of IO Ports [2]

tx_sdcard_7S:  
  lsl     r0, r0, #25   // Put Bit 7 in Carry.     [1]
  sd_tx_bit             // Send the Bit, CLK down. [7/6]

tx_sdcard_6S:
  lsl     r0, r0, #1    // Put Bit 6 in Carry.    [1]
  str     r1, [r3,  # PORT_OUTCLR_OFFSET]       
                        // Clear CLK              [2]
  sd_tx_bit             // Send the Bit, CLK down.[7/6]

tx_sdcard_5S:
  lsl     r0, r0, #1    // Put Bit 5 in Carry.
  str     r1, [r3,  # PORT_OUTCLR_OFFSET] 
                        // Clear CLK
  sd_tx_bit             // Send the Bit, CLK down.

tx_sdcard_4S:
  lsl     r0, r0, #1    // Put Bit 4 in Carry.
  str     r1, [r3,  # PORT_OUTCLR_OFFSET] 
                        // Clear CLK
  sd_tx_bit             // Send the Bit, CLK down.

tx_sdcard_3S:
  lsl     r0, r0, #1    // Put Bit 3 in Carry.
  str     r1, [r3,  # PORT_OUTCLR_OFFSET] 
                        // Clear CLK
  sd_tx_bit             // Send the Bit, CLK down.

tx_sdcard_2S:
  lsl     r0, r0, #1    // Put Bit 2 in Carry.
  str     r1, [r3,  # PORT_OUTCLR_OFFSET] 
                        // Clear CLK
  sd_tx_bit             // Send the Bit, CLK down.

tx_sdcard_1S:
  lsl     r0, r0, #1    // Put Bit 1 in Carry.
  str     r1, [r3,  # PORT_OUTCLR_OFFSET] 
                        // Clear CLK
  sd_tx_bit             // Send the Bit, CLK down.

tx_sdcard_0S:
  lsl     r0, r0, #1    // Put Bit 0 in Carry.
  str     r1, [r3,  # PORT_OUTCLR_OFFSET] 
                        // Clear CLK
  sd_tx_bit             // Send the Bit, CLK down.

  str     r1, [r3,  # PORT_OUTCLR_OFFSET]     
                        // Clear CLK      [2]

  bx  lr                // Return to Caller. [1]

.endfunc

#if 0
// Default Clock State is HIGH.
// CS is controlled by caller.

// Single Bit Cycle is:

// Set Write Bit
// Clock Low
// Get Read Bit
// Clock High

.func   TX_SPI_WRITE_BYTE
.global TC_SPI_WRITE_BYTE
TC_SPI_WRITE_BYTE:
        // Transfer a single byte over SPI 
        // to the Thermocouple.
        // Byte to send is in r0
  bx  lr
.endfunc

.func   TX_SPI_READ_BYTE
.global TC_SPI_READ_BYTE
TC_SPI_READ_BYTE:
        // Read a single byte over SPI 
        // from the Thermocouple.
  bx  lr
.endfunc

.func   TX_SPI_READ_WORD
.global TC_SPI_READ_WORD
TC_SPI_READ_WORD:
        // Read 32 bits over SPI from the Thermocouple.
  bx  lr
.endfunc
#endif

/*
 * Shared Constant Table
 */
.align 2                // 32 bit alignment
IOPortBase: .long PORT  // Base Address of Part Registers.
