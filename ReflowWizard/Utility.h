// Written by Peter Easton
// Released under CC BY-NC-SA 3.0 license
// Build a reflow oven: http://whizoo.com
//
#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stdint.h>

// Convert the duration into a readable string
void secondsToEnglishString(char *str, uint32_t seconds);

// Display seconds in the format hhh:mm:ss
char *secondsInClockFormat(char *str, uint32_t seconds);

// Animate the heating element and fan icons, based on their current state
// This function is called 50 times per second
void animateIcons(uint16_t x);

#endif