#include "atmel_asf4.h"
#include "debug_leds.h"

void vConfigureTimerForRunTimeStats(void)
{
}

uint32_t vGetRunTimeCounterValue(void)
{
	return 1;
}

void hwinit(void) 
{
	system_init();
	usb_init();
	dma_memory_init();

}

void subsystem_init(void)
{
	initDebugLeds();
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	hwinit();
	subsystem_init();

	/* Replace with your application code */
	vTaskStartScheduler();

	while (1) { 
		/* We only get here if the RTOS fails */ 
	}
}
