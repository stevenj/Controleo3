#include "atmel_asf4.h"
#include "debug_leds.h"
#include "usb_handler.h"

void vConfigureTimerForRunTimeStats(void)
{
}

uint32_t vGetRunTimeCounterValue(void)
{
	return 1;
}

void hw_init(void) 
{
	system_init();
	usb_init();
	dma_memory_init();

}

void subsystem_init(void)
{
	initDebugLeds();
	usb_init();
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	hw_init();
	subsystem_init();

	/* Replace with your application code */
	vTaskStartScheduler();

	while (1) { 
		/* We only get here if the RTOS fails */ 
	}
}
