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

#define millis() ((xTaskGetTickCount() * 1000)/configTICK_RATE_HZ)
#define delay(x) vTaskDelay(x/portTICK_PERIOD_MS)
void delayMicroseconds(uint16_t us);

extern const int pow10[10];
#define POW10(x) (x < 0? 0 : x <= 9? pow10[x] : pow10[9]) 


// Returns the CPU HZ / 256 which @ 48Mhz gives about 6 Hours of
// Runtime.
uint32_t vGetRunTimeCounterValue(void);

void vTaskPrintRunTimeStats(void);
void PrintMemoryStats(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTOS_START_H */
