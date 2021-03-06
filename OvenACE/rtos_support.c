/*
 * RTOS Support Code.
 *
 */
#include "atmel_asf4.h"
#include "rtos_support.h"
#include "HWPinAssignments.h"
#include "printf-stdarg.h"

static uint8_t overflow_hz;
static uint32_t last_hz_read;

const int pow10[10] = {
	1, 
	10, 
	100, 
	1000, 
	10000, 
	100000, 
	1000000, 
	10000000, 
	100000000, 
	1000000000
};

#if defined(DEBUG)
	static volatile int8_t      *bad_task_name;
	static volatile xTaskHandle *bad_task_handle;

	void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName )
	{
		bad_task_handle = pxTask;     // this seems to give me the crashed task handle
		bad_task_name = pcTaskName;     // this seems to give me a pointer to the name of the crashed task
		
		// Turn off the micro trace buffer so we don't fill it up in the infinite
		// loop below.
		REG_MTB_MASTER = 0x00000000 + 6;

		for( ;; );
	}
#endif

// Runtime stats are ALWAYS available.
void vConfigureTimerForRunTimeStats(void)
{
	// We use TC4:5 as a 32bit timer running at the CPU Frequency.
	// It has zero overhead, and gives an exact count of CPU cycles.
	// NOTE: Data datasheet says its 4:3 but its ACTUALLY 4:5.

	/* Enable the clocks in the PM */
	PM->APBCMASK.bit.TC4_ = true;
	PM->APBCMASK.bit.TC5_ = true;

    /* Set GCLK as source for TCC */
    GCLK_CLKCTRL_Type clkctrl = {0};
    clkctrl.bit.GEN   = CPU_CYCLE_TC_GCLK; // GP Clock (1.8Mhz)
    clkctrl.bit.CLKEN = true;
    clkctrl.bit.ID    = GCLK_CLKCTRL_ID_TC4_TC5_Val;
    GCLK->CLKCTRL     = clkctrl;

	// Reset BOTH TC's
	TC4->COUNT32.CTRLA.bit.SWRST = true;
	TC5->COUNT32.CTRLA.bit.SWRST = true;

	// Wait until the reset completes.
	while (TC4->COUNT32.STATUS.bit.SYNCBUSY) {}
	while (TC4->COUNT32.CTRLA.bit.SWRST) {}
	while (TC5->COUNT32.STATUS.bit.SYNCBUSY) {}
	while (TC5->COUNT32.CTRLA.bit.SWRST) {}

	// Set up so we can ALWAYS just read the current counter value.
	TC_READREQ_Type rrq = {0};
	rrq.bit.RCONT = true;
	rrq.bit.ADDR  = TC_COUNT32_COUNT_OFFSET;
	TC4->COUNT32.READREQ = rrq; 

	// Reset the counter.
	TC4->COUNT32.COUNT.reg = 0;

	// Configure and start the timer.  
    TC_CTRLA_Type tc_ctrl_a = {0};
	tc_ctrl_a.bit.ENABLE = true;
	tc_ctrl_a.bit.MODE = TC_CTRLA_MODE_COUNT32_Val;
	tc_ctrl_a.bit.WAVEGEN = TC_CTRLA_WAVEGEN_NFRQ_Val;
	tc_ctrl_a.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV1;
	tc_ctrl_a.bit.RUNSTDBY = false;
	tc_ctrl_a.bit.PRESCSYNC = TC_CTRLA_PRESCSYNC_GCLK_Val;
	TC4->COUNT32.CTRLA = tc_ctrl_a;

	overflow_hz = 0;
	last_hz_read  = 0;
}



uint32_t vGetRunTimeCounterValue(void)
{
	// Get RAW 32 bit CPU Hz Counter.
	uint32_t this_hz = CPU_HZ_COUNTER();

	// If we wrapped the CPU HZ counter, increment the overflow.
	// Works perfectly if this call is made at least once every
	// 90 seconds (and it is)
	if (this_hz < last_hz_read) {
		overflow_hz++;
	}

	// Save the last_hz count.
	last_hz_read = this_hz;

	// Divide raw CPU Clock by 256 and add overflow to the top.
	// Giving us about 6 hours of runtime before overflow.
	// Which is enough for the oven statistics.
	return ((this_hz >> 8) | (overflow_hz < 24));
}

void delayMicroseconds(uint16_t us)
{
  uint32_t start;
  uint32_t duration;
  uint32_t now;

  if (us == 0) {
	  return;
  }
  
  start    = CPU_HZ_COUNTER();

  // Sleep and block for delays greater than a millisecond.
  if (us >= 1000) {
	  delay(us/1000);
  }

  duration = ((uint64_t)us * configCPU_CLOCK_HZ) / 1000000;
  now      = CPU_HZ_COUNTER();

  // Spin with interrupts enabled for durations less than a millisecond.
  while ((now - start) < duration) {
	  now = CPU_HZ_COUNTER();
  }
}


const char * const TASK_STATE_NAMES[] = {
	[eRunning]   = "RUNNING",
	[eReady]     = "READY",
	[eBlocked]   = "BLOCKED",
	[eSuspended] = "SUSPENDED",
	[eDeleted]   = "DELETED",
	[eInvalid]   = "INVALID"
};

const char TaskTableHeader[]    = "  NAME     : NUMBER   : STATE     : PRIO  : RUNTIME              : STACK      : UNUSED STACK\n";
const char TaskTableUnderline[] = "-----------+----------+-----------+-------+----------------------+------------+--------------\n";
const char TaskTableFormat[]    = "  %8s : %08X : %9s : %2d/%2d : %10u(%3u.%03u) : 0x%08X : %10u \n";

void vTaskPrintRunTimeStats(void)
{
	TaskStatus_t *       pxTaskStatusArray;
	volatile UBaseType_t uxArraySize, x;
	uint32_t             ulTotalTime;
	uint64_t             percent1000;

#if (configUSE_TRACE_FACILITY != 1)
	{
#error configUSE_TRACE_FACILITY must also be set to 1 in FreeRTOSConfig.h to use vTaskPrintRunTimeStats().
	}
#endif

	/* Take a snapshot of the number of tasks in case it changes while this
	function is executing. */
	uxArraySize = uxTaskGetNumberOfTasks();

	/* Allocate an array index for each task.  NOTE!  If
	configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
	equate to NULL. */
	pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

	if (pxTaskStatusArray != NULL) {
		/* Generate the (binary) data. */
		uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalTime);

		printfD(TaskTableHeader);
		printfD(TaskTableUnderline);

		/* Create a human readable table from the binary data. */
		for (x = 0; x < uxArraySize; x++) {
			/* What percentage of the total run time has the task used?
			This will always be rounded down to the nearest integer.
			ulTotalRunTimeDiv100 has already been divided by 100. */
			//ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalTime;

			// Percentage of time to 3 fixed places.
			percent1000 = pxTaskStatusArray[x].ulRunTimeCounter;
			percent1000 *= 100000;
			percent1000 /= ulTotalTime;

			printfD(TaskTableFormat,
						pxTaskStatusArray[x].pcTaskName,
						(unsigned int)pxTaskStatusArray[x].xTaskNumber,
						TASK_STATE_NAMES[pxTaskStatusArray[x].eCurrentState],
						(int)pxTaskStatusArray[x].uxCurrentPriority,
						(int)pxTaskStatusArray[x].uxBasePriority,
						(unsigned int)pxTaskStatusArray[x].ulRunTimeCounter,
						(unsigned int)percent1000/1000,
						(unsigned int)percent1000%1000,
						(unsigned int)pxTaskStatusArray[x].pxStackBase,
						(unsigned int)pxTaskStatusArray[x].usStackHighWaterMark
						);
		}
		printfD(TaskTableUnderline);

		printfD(TaskTableFormat,
					"TOTALS",
					(unsigned int)uxArraySize,
					"",
					0,
					0,
					(unsigned int)ulTotalTime,
					100,0, //%
					0,
					0
					);

		/* Free the array again.  NOTE!  If configSUPPORT_DYNAMIC_ALLOCATION
		is 0 then vPortFree() will be #defined to nothing. */
		vPortFree(pxTaskStatusArray);

	}
}

// Linker Symbols for memory statistics
extern uint32_t _srelocate;
extern uint32_t _ebss;
extern uint32_t _sheap;
extern uint32_t _eheap;
extern uint32_t _sstack;
extern uint32_t _estack;

#define IRQ_STACK_EVEN_FILL (0x53515249)
#define IRQ_STACK_ODD_FILL (0x4b434154)

static uint32_t IRQStackFree(void)
{
	uint32_t *start = &_sstack;
	uint32_t *end   = &_estack;

	while (start != end) {
		if (((uint32_t)start & 0x4) == 0) {
			if (*start != IRQ_STACK_EVEN_FILL) {
				break;
			}
		} else {
			if (*start != IRQ_STACK_ODD_FILL) {
				break;
			}
		}
		start++;
	}

	return ((uint32_t)(start - &_sstack))*sizeof(uint32_t);
}

void PrintMemoryStats(void)
{
	printfD("  ALLOCATED DATA :\n");
	printfD("    Start    = 0x%08X\n", (unsigned)&_srelocate);
	printfD("    End      = 0x%08X\n", (unsigned)&_ebss);
	printfD("    Size     = %u\n"    , (unsigned)(&_ebss - &_srelocate));
	printfD("\n");

	printfD("  HEAP :\n");
	printfD("    Start    = 0x%08X\n", (unsigned)&_sheap);
	printfD("    End      = 0x%08X\n", (unsigned)&_eheap);
	printfD("    Size     = %u\n"    , (unsigned)configTOTAL_HEAP_SIZE);
	printfD("    Free     = %u\n"    , xPortGetFreeHeapSize());
	printfD("    Min Free = %u\n"    , xPortGetMinimumEverFreeHeapSize());
	printfD("\n");

	printfD("  IRQ STACK :\n");
	printfD("    Top      = 0x%08X\n", (unsigned)&_estack);
	printfD("    Current  = 0x%08X\n", (unsigned)__get_MSP());
	printfD("    Bottom   = 0x%08X\n", (unsigned)&_sstack);
	printfD("    Size     = %u\n"    , ((unsigned)(&_estack - &_sstack))*sizeof(uint32_t));
	printfD("    Free     = %u\n"    , (unsigned)IRQStackFree());
}

