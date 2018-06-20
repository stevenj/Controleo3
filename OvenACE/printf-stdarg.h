#ifndef __PRINTF_STDARG_H__
#define __PRINTF_STDARG_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Debug version of printf.  Prints to the "DEBUG" stream.
int printfD(const char *format, ...)
    __attribute__ ((format (printf, 1, 2)));

#ifdef __cplusplus
}
#endif // __cplusplus

#endif