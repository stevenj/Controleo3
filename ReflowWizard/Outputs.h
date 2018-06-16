#ifndef __OUTPUTS_H__
#define __OUTPUTS_H__

#include <Arduino.h>

extern volatile uint32_t *portAOut, *portAMode, *portBOut, *portBMode;

// Initialize the registers controlling the outputs, and turn them off.
void initOutputs(void);

// Turn an output on or off
void setOutput(uint8_t outputNumber, boolean state);

// Get the current state of an output
boolean getOutput(uint8_t outputNumber);

// Turn elements and fans on or off in one go
void setOvenOutputs(boolean elementsOn, boolean convectionFanOn, boolean coolingFanOn);

// Turn the convection fan on or off
void turnConvectionFanOn(boolean on);

// Turn the cooling fan on or off
void turnCoolingFanOn(boolean on);

// Check to see if outputs are configured.  Prevent bake/reflow if not
// At least 2 elements must be heating elements
boolean areOutputsConfigured(void);

// See how many outputs are used
uint8_t numOutputsConfigured(void);


#endif