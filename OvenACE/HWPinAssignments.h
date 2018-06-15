/*
 * All Hardware Pin Assignments.
 */
#ifndef __HWPINASSIGNMENTS__H__
#define __HWPINASSIGNMENTS__H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "SimplePIO.h"

// DEBUG LEDS
#ifndef DEBUG  // Only defined when not a DEBUG build
    #define DEBUG_LED_RED   PA(30)
    #define DEBUG_LED_GREEN PA(31)
#else
    #define DEBUG_LED_RED   PB(11)      // Temporary for testing.
#endif


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // USB_DEVICE_MAIN_H
