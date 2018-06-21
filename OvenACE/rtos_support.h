/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */
#ifndef RTOS_START_H
#define RTOS_START_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <hal_rtos.h>

// Read RAW CPU HZ Counter.
// AT 48Mhz this will wrap in about 89 Seconds of run time.
#define CPU_HZ_COUNTER() (TC4->COUNT32.COUNT.reg)

// Returns the CPU HZ / 256 which @ 48Mhz gives about 6 Hours of
// Runtime.
uint32_t vGetRunTimeCounterValue(void);

void vTaskPrintRunTimeStats(void);
void PrintMemoryStats(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTOS_START_H */
