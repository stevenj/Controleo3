/*
 * All Hardware Pin Assignments.
 */
#ifndef __PIEZO_BUZZER__H__
#define __PIEZO_BUZZER__H__

#include "HWPinAssignments.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define HZ_TO_FREQ(hz) (GCLK3_HZ/hz)

// Flash a particular debug led, a particular number of times.
void PlayTone(uint16_t freq);

void initPiezoBuzzer(void);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // USB_DEVICE_MAIN_H
