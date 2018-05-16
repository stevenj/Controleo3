#ifndef __PREFS_H__
#define __PREFS_H__

#include <Arduino.h>

void getPrefs(void);

// Save the prefs to external flash
void savePrefs(void);

// This is called while waiting for the user to tap the screen
void checkIfPrefsShouldBeWrittenToFlash(void);

// Flash writes are good for 50,000 cycles - and there are 4 blocks used for prefs = 200,000 cycles.  
void writePrefsToFlash(void);

// This performs a factory reset, erasing preferences and profiles
void factoryReset(boolean saveTouchCalibrationData);

#endif