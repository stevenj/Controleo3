#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include <stdint.h>

// Initialize the MAX31856's registers
void initTemperature(void);

// This function is called every 200ms from the Timer 1 (servo) interrupt
void takeCurrentThermocoupleReading(void);

// Routine used by the main app to get temperatures
float getCurrentTemperature(void);

// Convert the temperature to a string
char *getTemperatureString(char *str, float temperature, bool displayInCelsius);

#endif