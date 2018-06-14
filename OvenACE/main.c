#include "atmel_asf4.h"

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

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	hwinit();
	FREERTOS_V1000_0_example();

	/* Replace with your application code */
	while (1) {
	}
}
