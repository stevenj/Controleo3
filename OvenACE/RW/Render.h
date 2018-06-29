#ifndef __RENDER_H__
#define __RENDER_H__

#include <stdint.h>

uint16_t renderBitmap(uint16_t bitmapNumber, uint16_t x, uint16_t y);

// Render a bitmap from external flash
// Returns the width of the rendered bitmap (needed when writing text)
uint16_t renderBitmapFromExternalFlash(uint16_t bitmapNumber, uint16_t x, uint16_t y);

// Render a bitmap from microcontroller flash
// Returns the width of the rendered bitmap (needed when writing text)
// The first 2 bytes of the bitmap is the width and height of the bitmap
uint16_t renderBitmapFromMicrocontrollerFlash(uint16_t bitmapNumber, uint16_t x, uint16_t y);

// Display a string on the screen, using the specified font
// Only ASCII-printable character are supported
uint16_t displayString(uint16_t x, uint16_t y, uint8_t font, char *str);

// Display a character on the screen, using the specified font
uint16_t displayCharacter(uint8_t font, uint16_t x, uint16_t y, uint8_t c);

// The whitespace around each character is not stored as part of the bitmap, to make
// rendering as fast as possible.  The result is that all characters are not at the same
// height so an offset is necessary.
int16_t getYOffsetForCharacter(uint8_t font, uint8_t c);

// Determine if the character is in the font set
// 9 and 12-point fonts contain all printable characters
// The 22-point font contains a small subset
bool isSupportedCharacter(uint8_t font, uint8_t c);

uint16_t preCharacterSpace(uint8_t font, char c);

uint16_t postCharacterSpace(uint8_t font, char c) ;

// Display a fixed-width string (typically a number that keeps getting updated)
void displayFixedWidthString(uint16_t x, uint16_t y, char *str, uint8_t maxChars, uint8_t font);

// Draw a button on the screen
void drawButton(uint16_t x, uint16_t y, uint16_t width, uint16_t textWidth, bool useLargeFont, char *text);

void drawButtonOutline(uint16_t x, uint16_t y, uint16_t width);

#endif