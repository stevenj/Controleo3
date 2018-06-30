#include "atmel_asf4.h"
#include "debug_leds.h"
#include "piezo_buzzer.h"
#include "usb_handler.h"


void hw_init(void) 
{
	system_init();
	dma_memory_init();
	usb_init();
}

void subsystem_init(void)
{
	initDebugLeds();
	initPiezoBuzzer();
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	hw_init();
	subsystem_init();

	//PlayTone(HZ_TO_FREQ(261.626));


	/* Replace with your application code */
	vTaskStartScheduler();

	while (1) { 
		/* We only get here if the RTOS fails */ 
	}
}
