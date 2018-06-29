#ifndef __SERVO_H__
#define __SERVO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Starts the timer.  Called on startup
void initializeTimer(void);

// Interrupt handler for TC3
void TC3_Handler(void);

// Move the servo to servoDegrees, in timeToTake milliseconds (1/1000 second)
void setServoPosition(uint8_t servoDegrees, uint16_t timeToTake);

// Convert degrees (0-180) to a timer counter value
uint16_t degreesToTimerCounter(uint8_t servoDegrees);

#ifdef __cplusplus
}
#endif

#endif