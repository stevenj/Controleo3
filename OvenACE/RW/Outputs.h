#ifndef __OUTPUTS_H__
#define __OUTPUTS_H__

#include <stdint.h>

extern volatile uint32_t *portAOut, *portAMode, *portBOut, *portBMode;

// Initialize the registers controlling the outputs, and turn them off.
void initOutputs(void);

// Turn an output on or off
void setOutput(uint8_t outputNumber, bool state);

// Get the current state of an output
bool getOutput(uint8_t outputNumber);

// Turn elements and fans on or off in one go
void setOvenOutputs(bool elementsOn, bool convectionFanOn, bool coolingFanOn);

// Turn the convection fan on or off
void turnConvectionFanOn(bool on);

// Turn the cooling fan on or off
void turnCoolingFanOn(bool on);

// Check to see if outputs are configured.  Prevent bake/reflow if not
// At least 2 elements must be heating elements
bool areOutputsConfigured(void);

// See how many outputs are used
uint8_t numOutputsConfigured(void);


#endif