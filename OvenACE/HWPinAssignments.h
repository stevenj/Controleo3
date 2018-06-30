/*
 * All Hardware Pin (and other) Assignments.
 */
#ifndef __HWPINASSIGNMENTS__H__
#define __HWPINASSIGNMENTS__H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "SimplePIO.h"

/* ********* GCLK CONFIGURATION ******************* */
#define GCLK0_HZ (CONF_CPU_FREQUENCY)
#define GCLK1_HZ (8000000)
#define GCLK2_HZ (32768)
#define GCLK3_HZ (1800000)
// GCLK 4-7 UNUSED
#define GCLK_HZ(x) (GCLK ## x ## _HZ)

/* ********* CPU CYCLE TIMER ******************* */
#define CPU_CYCLE_TC_MASTER 4
#define CPU_CYCLE_TC_SLAVE  5
#define CPU_CYCLE_TC_GCLK   0

/* ********* DEBUG LEDS ******************* */
#ifndef DEBUG  // Only defined when not a DEBUG build
    #define DEBUG_LED_RED   PA(30)
    #define DEBUG_LED_GREEN PA(31)
#else
    #define DEBUG_SWCLK     PA(30)
    #define DEBUG_SWDIO     PA(31)
    #define DEBUG_LED_RED   PB(11)      // Temporary for testing.
#endif

/* ********* PIEZO BUZZER ******************* */
#define PIEZO_BUZZER PA(12)
#define PIEZO_TCC    2
#define PIEZO_GCLK   3

/* ********* RELAY OUTPUTS ******************* */
#define RELAY_OUTPUT1 PA(15)
#define RELAY_OUTPUT2 PB(30)
#define RELAY_OUTPUT3 PB(17)
#define RELAY_OUTPUT4 PB(9)
#define RELAY_OUTPUT5 PB(8)

/* ********* A/C ZERO CROSS INPUT ******************* */
#define ZERO_CROSS_INPUT PB(11)
#define ZERO_CROSS_INT   EXTINT(11)

/* ********* SERVO PWM OUTPUT ******************* */
#define SERVO_OUTPUT PB(31)
#define SERVO_TCC    0

/* ********* THERMOCOUPLE IC ******************* */
#define THERMOCOUPLE_MISO PA(20)
#define THERMOCOUPLE_MOSI PA(21)
#define THERMOCOUPLE_CLK  PA(22)
#define THERMOCOUPLE_CS   PA(23)

/* ********* SDCARD INTERFACE (SPI) ******************* */
#define USE_HW_SPI        0
#define SDCARD_CD         PA(2)
#define SDCARD_MISO       PA(4)
#define SDCARD_CLK        PA(5)
#define SDCARD_SERCOM     0
#define SDCARD_GCLK       0

#if USE_HW_SPI
    // We could use ALL hardware if we mod the board to swap MOSI/CS
    #define SDCARD_MOSI   PA(7)
    #define SDCARD_CS     PA(6)
#else
    #define SDCARD_MOSI   PA(6)
    #define SDCARD_CS     PA(7)
#endif

/* ********* LCD INTERFACE ******************* */
#define LCD_D(x)  PB(x)
#define LCD_RD    PB(12)
#define LCD_WR    PB(13)
#define LCD_DCX   PB(14)
#define LCD_CS    PB(15)
#define LCD_RESET PB(16)

/* ********* TOUCHSCREEN INTERFACE ******************* */
#define TS_CLK    PA(8)
#define TS_CS     PA(9)
#define TS_MOSI   PA(10)
#define TS_MISO   PA(11)
#define TS_IRQ    PB(10)
#define TS_INT    EXTINT(10)

/* ********* ONBOARD FLASH ******************* */
#define FLASH_CLK  PA(13)
#define FLASH_CS   PA(14)
#define FLASH_MOSI PA(16)
#define FLASH_MISO PA(17)
#define FLASH_WP   PA(18)
#define FLASH_HOLD PA(19)

/* ********* USB ******************* */
#define USB_DM      PA(24)
#define USB_DP      PA(25)


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // USB_DEVICE_MAIN_H
