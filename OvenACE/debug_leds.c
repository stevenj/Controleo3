/*
 * Debug LED Handler
 *
 * Our debug leds share resources with the SWD Port.  So, for debug builds we
 * don't actually do anything.
 */
#include "atmel_asf4.h"
#include "debug_leds.h"

#include "SimplePIO.h"
#include "HWPinAssignments.h"
#include "usb_handler.h"
#include "printf-stdarg.h"
#include "stdio.h"

#define DEBUGTASK_STACK_SIZE (128)
#define DEBUGTASK_PRIORITY   (tskIDLE_PRIORITY + 1)

static TaskHandle_t      xDebugLedTask;

typedef struct {
  uint8_t repeats;  // count, 0 = Off
  uint8_t on_time;  // 100 ms
  uint8_t off_time; // 100 ms
} DebugStatus;

#define DBGTIME_MS(x) (x/100)
#define DBG_HB_MAXTIME 1000

// Status Flashes are for the Green Led
DebugStatus DebugStates[DEBUG_HB_REFRESH] = {
	{5,  DBGTIME_MS(2000), DBGTIME_MS(2000)}, // DEBUG_HB_FAILED
	{2,  DBGTIME_MS( 100), DBGTIME_MS( 100)}, // DEBUG_LED_STARTED
	{10, DBGTIME_MS(1000), DBGTIME_MS(500)},
	{20, DBGTIME_MS( 100), DBGTIME_MS(100)},
};

static DebugStatus CurrentDebugState; 
static uint8_t     HBRepeat;
static uint32_t    dbgcnt;

/**
 * Debug LED Handler
 *
 * \param[in] p The void pointer for OS task Standard model.
 *
 */
static void DebugLed_task(void *p)
{
	(void)p; // Unused
	int cnt = 0;
	int mcnt = 0;

	while (1) {
  	  #ifdef DEBUG_LED_RED
		if (HBRepeat > 0) {
			HBRepeat -= 1;
			PORT_OUTTGL(DEBUG_LED_RED) = PBITRAW(DEBUG_LED_RED);
		} else {
			PORT_OUTSET(DEBUG_LED_RED) = PBITRAW(DEBUG_LED_RED);
		}
	  #endif	
	  #ifdef DEBUG_LED_GREEN
		if (CurrentDebugState.repeats != 0) {
			dbgcnt++;
			if (dbgcnt == 1) {
				PORT_OUTSET(DEBUG_LED_GREEN) = PBITRAW(DEBUG_LED_GREEN);
			} else if (dbgcnt == CurrentDebugState.on_time) {
				PORT_OUTCLR(DEBUG_LED_GREEN) = PBITRAW(DEBUG_LED_GREEN);
			} else if (dbgcnt == CurrentDebugState.on_time + CurrentDebugState.off_time) {
				CurrentDebugState.repeats--;
				dbgcnt = 0;
			}
		}
	  #endif	
		os_sleep(pdMS_TO_TICKS(100));

		cnt++;
		if (cnt == 100) {
			printf("Message %d 234567 20 234567 30 234567 40 234567 50 234567 60 234567 70 234567 80\n", mcnt);
			mcnt++;
		} else if (cnt == 200) {
			printfD("Message %d ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$^&*()-={}|[];':,./<>?`~\n", mcnt);
			mcnt++;
			cnt = 0;
		}

	}
}

// Flash a particular debug led, a particular number of times.
void DebugLedFlash(DebugLedState state) 
{
	if (state == DEBUG_HB_REFRESH ) {
		HBRepeat = DBGTIME_MS(DBG_HB_MAXTIME); 
		               // Reset count of Debug Heartbeat.  
		               // Heartbeat will stop if not called often enough.
	} else {
		CurrentDebugState = DebugStates[state];
		dbgcnt = 0;
	}

}

void initDebugLeds(void)
{
	// Port Configuration - should colapse to a constant definition.
	PORT_PINCFG_Type cfg = {0};
	cfg.bit.DRVSTR = true;
	cfg.bit.INEN   = false;
	cfg.bit.PULLEN = false;
	cfg.bit.PMUXEN = false;

	// Initialise the LEDS as Outputs, HIGH Drive.
	#ifdef DEBUG_LED_RED
		PORT_PINCFG(DEBUG_LED_RED) = cfg;
	#endif

	#ifdef DEBUG_LED_GREEN
		PORT_PINCFG(DEBUG_LED_GREEN) = cfg;
	#endif

	#if defined(DEBUG_LED_RED) && defined(DEBUG_LED_GREEN)
		PORT_OUTPUT(PBIT(DEBUG_LED_RED) | PBIT(DEBUG_LED_GREEN));
	#elif defined(DEBUG_LED_RED)
		PORT_OUTPUT(PBIT(DEBUG_LED_RED));
	#elif defined(DEBUG_LED_GREEN)
		PORT_OUTPUT(PBIT(DEBUG_LED_GREEN));
	#endif

	// Initialise State.
	DebugLedFlash(DEBUG_HB_REFRESH);
	DebugLedFlash(DEBUG_LED_STARTED);

	// Create task which handles flashing the leds
	xTaskCreate(
        DebugLed_task, "DBUG Led", 
        DEBUGTASK_STACK_SIZE, NULL, 
        DEBUGTASK_PRIORITY, &xDebugLedTask);
}
