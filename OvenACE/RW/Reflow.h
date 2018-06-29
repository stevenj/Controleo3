#ifndef __REFLOW_H__
#define __REFLOW_H__

#include <stdint.h>

void reflow(uint8_t profileNo);

// Draw the abort dialog on the screen.  The user needs to confirm that they want to exit reflow
void drawReflowAbortDialog(void);

void updateStatusMessage(uint16_t token, uint16_t timer, uint16_t temperature);

// Show an error dialog when an error forces reflow to be aborted
void showReflowError(uint16_t iconsX, char *line1, char *line2);

// Display the reflow timer
void displayReflowDuration(uint32_t seconds);

// Calculate the expected power level based on the desired temperature and desired rate-of-rise
uint16_t getBasePIDPower(double temperature, double increment, uint16_t *bias, uint16_t maxBias);

#endif