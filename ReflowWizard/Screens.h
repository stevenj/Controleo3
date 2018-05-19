#ifndef __SCREENS_H__
#define __SCREENS_H__

#include <Arduino.h>

void setTouchTemperatureUnitChangeCallback(void (*f) (boolean));

// This is the main loop, displaying one screen after the other as the user
// navigates between them.
void showScreen(uint8_t screen);

void drawTouchButton(uint16_t x, uint16_t y, uint16_t width, uint16_t textWidth, boolean useLargeFont, char *text);

// Draw the naviation buttons at the bottom of the screen
// Use large tap targets if possible
void drawNavigationButtons(boolean addRightArrow, boolean largeTargets);

// Display the screen title
void displayHeader(char *text, boolean isSetting);

void drawThickRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t thickness, uint16_t color);

// Draw arrow for increasing / decreasing settings - and add tap targets for them
void drawIncreaseDecreaseTapTargets(uint8_t targetType);

// Used when user is testing their relays
void drawTestOutputIcon(uint8_t type, boolean outputIsOn);

void testOutputIconAnimator(void);

#endif