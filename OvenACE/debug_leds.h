/*
 * All Hardware Pin Assignments.
 */
#ifndef __DEBUG_LEDS__H__
#define __DEBUG_LEDS__H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum {
    DEBUG_HB_FAILED = 0,
    DEBUG_LED_STARTED,
    DEBUG_FLASH1,
    DEBUG_FLASH2,
    DEBUG_HB_REFRESH, // ALWAYS last entry in enum.
} DebugLedState;

// Flash a particular debug led, a particular number of times.
void DebugLedFlash(DebugLedState state);

void initDebugLeds(void);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // USB_DEVICE_MAIN_H
