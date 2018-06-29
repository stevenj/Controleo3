#ifndef __HELP_H__
#define __HELP_H__

#include <stdint.h>

void showHelp(uint8_t screen);

void drawHelpBorder(uint16_t width, uint16_t height);

void eraseHelpScreen(uint16_t width, uint16_t height);

void displayHelpLine(char *text);

#endif