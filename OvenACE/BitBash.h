#ifndef __BITBASH_H__
#define __BITBASH_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

uint8_t TC_SPI_ONE(uint8_t tx);

void TX_SDCARD(uint8_t byte);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif