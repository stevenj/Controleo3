#ifndef __LEARN_H__
#define __LEARN_H__

#include <stdint.h>

// Stay in this function until learning is done or canceled
void learn(void);
// Print baking information to the serial port so it can be plotted
void DisplayLearningTime(uint16_t duration, float temperature, int duty, int integral);

// Draw the abort dialog on the screen.  The user needs to confirm that they want to exit bake
void drawLearningAbortDialog(void);

// Display the countdown timer
void displaySecondsLeft(uint32_t overallSeconds, uint32_t phaseSeconds);

// Draw the performance graph to indicate instantaneous performance
void drawPerformanceBar(bool redraw, uint8_t percentage);

// Calculate the overall oven score
uint8_t ovenScore(void);

// The learned numbers are shown once the oven has completed the 1-hour learning run
void showLearnedNumbers(void);

#endif
