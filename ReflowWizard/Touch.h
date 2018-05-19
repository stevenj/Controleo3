#ifndef __TOUCH_H_
#define __TOUCH_H_

#include <Arduino.h>

// Calibrate the touch screen
void CalibrateTouchscreen(void);

// Draw the calibration crosshairs on the screen
void drawCrosshairs(uint16_t x, uint16_t y, boolean draw);

// Debounce touches, but allow for tap-and-hold
void debounce(void);

// Debounce, but with smaller times (used for non-critical changes, like changing C to F)
void quickDebounce(void);

// Wait for 250ms after the last touch
void calibrationDebounce(void);

// Send the calibration data to the touch driver
void sendTouchCalibrationData(void);

// Clear all tap data
void clearTouchTargets(void);

// Callback function to call while waiting for touch.  Interval is in milliseconds
void setTouchIntervalCallback(void (*f) (), uint16_t interval);

// Callback function when user taps in top-right corner to change temperature units
void setTouchTemperatureUnitChangeCallback(void (*f) (boolean displayInCelsius));

void defineTouchArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

int8_t getTap(uint8_t mode);

void touchCallback(void);

// Display the temperature on the screen once per second
void displayTemperatureInHeader(void);

#endif