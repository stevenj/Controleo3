#ifndef __ARDUINO_DEFS__
#define __ARDUINO_DEFS__

#include "FreeRTOS.h"
// These ALL need to be re-implemented, only included here for expediency now
#define digitalRead(X) (1)
#define digitalWrite(X, Y) {}
#define setWriteError() {}
#define pinMode(x, y) {}
#define pgm_read_word(x) (1)
#define tone(x,y,z) {}
#define noTone(x) {}
#define PORTB (1)

#define highByte(x) ((x>>8) & 0xFF)
#define lowByte(x) (x & 0xFF)

// We may need to keep these, and if we do move to rtos_support.h
static inline long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline long constrain(long x, long a, long b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

static inline long abs (long x) {
  if (x >= 0) return x;
  return 0-x;
}

static inline void* malloc(size_t size)
{
    void* ptr = NULL;

    if(size > 0)
    {
        // We simply wrap the FreeRTOS call into a standard form
        ptr = pvPortMalloc(size);
    } // else NULL if there was an error

    return ptr;
}

static inline void free(void* ptr)
{
    if(ptr)
    {
        // We simply wrap the FreeRTOS call into a standard form
        vPortFree(ptr);
    }
}
#endif