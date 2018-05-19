#ifndef __BAKE_H__
#define __BAKE_H__

#include <Arduino.h>

// Stay in this function until the bake is done or canceled
void bake(void);

// Print baking information to the serial port so it can be plotted
void DisplayBakeTime(uint16_t duration, float temperature, int duty, int integral);

// Display the baking phase on the screen
void displayBakePhase(uint8_t phase, boolean abortDialogIsOnScreen);

// Draw the abort dialog on the screen.  The user needs to confirm that they want to exit bake
void drawBakingAbortDialog(void);

// Display the bake temperature and duration on the screen
// This is also a callback routine, called if the user taps in the top-right corner
void displayBakeTemperatureAndDuration(boolean displayCelsius);

// Display the countdown timer
void displayBakeSecondsLeft(uint32_t seconds);

// Returns the bake duration, in seconds
uint32_t getBakeSeconds(uint16_t duration);

#endif