/**
 * \file
 *
 * \brief gcc starttup file for SAMD21
 *
 * Copyright (c) 2018 Microchip Technology Inc.
 *
 * \asf_license_start
 *
 * \page License
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the Licence at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \asf_license_stop
 *
 */

#include "samd21.h"

/* Initialize segments */
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sheap;
extern uint32_t _eheap;
extern uint32_t _sstack;
extern uint32_t _estack;


#if defined(DEBUG)
    // Create a Trace Buffer
    #define TRACE_BUFFER_SIZE 256
    __attribute__((__aligned__(TRACE_BUFFER_SIZE * sizeof(uint32_t)))) uint32_t mtb[TRACE_BUFFER_SIZE];
#endif

/** \cond DOXYGEN_SHOULD_SKIP_THIS */
int main(void);
/** \endcond */

void __libc_init_array(void);

/* Default empty handler */
void Dummy_Handler(void);

/* Cortex-M0+ core handlers */
void NonMaskableInt_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HardFault_Handler(void) __attribute__((weak)); // Don't make it the Dummy Handler.
void SVCall_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Dummy_Handler")));

/* Peripherals handlers */
void PM_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SYSCTRL_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void WDT_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void RTC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void EIC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void NVMCTRL_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void DMAC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void USB_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void EVSYS_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM3_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM4_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM5_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TCC0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TCC1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TCC2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC3_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC4_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC5_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC6_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC7_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void ADC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void AC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void DAC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PTC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void I2S_Handler(void) __attribute__((weak, alias("Dummy_Handler")));

/* Exception Table */
__attribute__((section(".vectors"))) const DeviceVectors exception_table = {

    /* Configure Initial Stack Pointer, using linker-generated symbols */
    .pvStack = (void *)(&_estack),

    .pfnReset_Handler          = (void *)Reset_Handler,
    .pfnNonMaskableInt_Handler = (void *)NonMaskableInt_Handler,
    .pfnHardFault_Handler      = (void *)HardFault_Handler,
    .pvReservedM12             = (void *)(0UL), /* Reserved */
    .pvReservedM11             = (void *)(0UL), /* Reserved */
    .pvReservedM10             = (void *)(0UL), /* Reserved */
    .pvReservedM9              = (void *)(0UL), /* Reserved */
    .pvReservedM8              = (void *)(0UL), /* Reserved */
    .pvReservedM7              = (void *)(0UL), /* Reserved */
    .pvReservedM6              = (void *)(0UL), /* Reserved */
    .pfnSVCall_Handler         = (void *)SVCall_Handler,
    .pvReservedM4              = (void *)(0UL), /* Reserved */
    .pvReservedM3              = (void *)(0UL), /* Reserved */
    .pfnPendSV_Handler         = (void *)PendSV_Handler,
    .pfnSysTick_Handler        = (void *)SysTick_Handler,

    /* Configurable interrupts */
    .pfnPM_Handler      = (void *)PM_Handler,      /*  0 Power Manager */
    .pfnSYSCTRL_Handler = (void *)SYSCTRL_Handler, /*  1 System Control */
    .pfnWDT_Handler     = (void *)WDT_Handler,     /*  2 Watchdog Timer */
    .pfnRTC_Handler     = (void *)RTC_Handler,     /*  3 Real-Time Counter */
    .pfnEIC_Handler     = (void *)EIC_Handler,     /*  4 External Interrupt Controller */
    .pfnNVMCTRL_Handler = (void *)NVMCTRL_Handler, /*  5 Non-Volatile Memory Controller */
    .pfnDMAC_Handler    = (void *)DMAC_Handler,    /*  6 Direct Memory Access Controller */
    .pfnUSB_Handler     = (void *)USB_Handler,     /*  7 Universal Serial Bus */
    .pfnEVSYS_Handler   = (void *)EVSYS_Handler,   /*  8 Event System Interface */
    .pfnSERCOM0_Handler = (void *)SERCOM0_Handler, /*  9 Serial Communication Interface 0 */
    .pfnSERCOM1_Handler = (void *)SERCOM1_Handler, /* 10 Serial Communication Interface 1 */
    .pfnSERCOM2_Handler = (void *)SERCOM2_Handler, /* 11 Serial Communication Interface 2 */
    .pfnSERCOM3_Handler = (void *)SERCOM3_Handler, /* 12 Serial Communication Interface 3 */
    .pfnSERCOM4_Handler = (void *)SERCOM4_Handler, /* 13 Serial Communication Interface 4 */
    .pfnSERCOM5_Handler = (void *)SERCOM5_Handler, /* 14 Serial Communication Interface 5 */
    .pfnTCC0_Handler    = (void *)TCC0_Handler,    /* 15 Timer Counter Control 0 */
    .pfnTCC1_Handler    = (void *)TCC1_Handler,    /* 16 Timer Counter Control 1 */
    .pfnTCC2_Handler    = (void *)TCC2_Handler,    /* 17 Timer Counter Control 2 */
    .pfnTC3_Handler     = (void *)TC3_Handler,     /* 18 Basic Timer Counter 0 */
    .pfnTC4_Handler     = (void *)TC4_Handler,     /* 19 Basic Timer Counter 1 */
    .pfnTC5_Handler     = (void *)TC5_Handler,     /* 20 Basic Timer Counter 2 */
    .pfnTC6_Handler     = (void *)TC6_Handler,     /* 21 Basic Timer Counter 3 */
    .pfnTC7_Handler     = (void *)TC7_Handler,     /* 22 Basic Timer Counter 4 */
    .pfnADC_Handler     = (void *)ADC_Handler,     /* 23 Analog Digital Converter */
    .pfnAC_Handler      = (void *)AC_Handler,      /* 24 Analog Comparators */
    .pfnDAC_Handler     = (void *)DAC_Handler,     /* 25 Digital Analog Converter */
    .pfnPTC_Handler     = (void *)PTC_Handler,     /* 26 Peripheral Touch Controller */
    .pfnI2S_Handler     = (void *)I2S_Handler,     /* 27 Inter-IC Sound Interface */
    .pvReserved28       = (void *)(0UL)            /* 28 Reserved */
};

/**
 * \brief This is the code that gets called on processor reset.
 * To initialize the device, and call the main() routine.
 */
void Reset_Handler(void)
{
	uint32_t *pSrc, *pDest;

	/* Initialize RAM Data by copying values from Flash to RAM */
	pSrc  = &_etext;
	pDest = &_srelocate;

	if (pSrc != pDest) {
		for (; pDest < &_erelocate;) {
			*pDest++ = *pSrc++;
		}
	}

	/* Clear the zero segment */
	for (pDest = &_szero; pDest < &_ezero;) {
		*pDest++ = 0;
	}

    // Clear the HEAP (Not with ZERO)
	pSrc  = &_sheap;
	pDest = &_eheap;
    while (pSrc < pDest) {
        *pSrc++ = 0x50414548; // String "HEAP"
    }


    #define IRQ_STACK_EVEN_FILL (0x53515249)
    #define IRQ_STACK_ODD_FILL (0x4b434154)

    // Fill the Boot Stack with 0x7e57ab1e and 0x7e117a1e
    // "IRQSTACK" (Visible in a memory hex dump)
    // So we can detect maximum IRQ stack usage and overflow.
	pSrc  = &_sstack;
	pDest = &_estack;
    while (pSrc < pDest) {
        if (((uint32_t)pSrc & 0x4) == 0) {
            *pSrc++ = IRQ_STACK_EVEN_FILL;
        } else {
            *pSrc++ = IRQ_STACK_ODD_FILL;
        }
    }

	/* Set the vector table base address */
	pSrc      = (uint32_t *)&_sfixed;
	SCB->VTOR = ((uint32_t)pSrc & SCB_VTOR_TBLOFF_Msk);

	/* Change default QOS values to have the best performance and correct USB behaviour */
	SBMATRIX->SFR[SBMATRIX_SLAVE_HMCRAMC0].reg = 2;
#if defined(ID_USB)
	USB->DEVICE.QOSCTRL.bit.CQOS = 2;
	USB->DEVICE.QOSCTRL.bit.DQOS = 2;
#endif
	DMAC->QOSCTRL.bit.DQOS   = 2;
	DMAC->QOSCTRL.bit.FQOS   = 2;
	DMAC->QOSCTRL.bit.WRBQOS = 2;

	/* Overwriting the default value of the NVMCTRL.CTRLB.MANW bit (errata reference 13134) */
	NVMCTRL->CTRLB.bit.MANW = 1;

	/* Initialize the C library */
	__libc_init_array();

#if defined(DEBUG)
    REG_MTB_POSITION = ((uint32_t) (mtb - REG_MTB_BASE)) & 0xFFFFFFF8;
    REG_MTB_FLOW = ((uint32_t) mtb + TRACE_BUFFER_SIZE * sizeof(uint32_t)) & 0xFFFFFFF8;
    REG_MTB_MASTER = 0x80000000 + 6;
#endif

	/* Branch to main function */
	main();

	/* Infinite loop */
	while (1)
		;
}

/**
 * \brief Default interrupt handler for unused IRQs.
 */
void Dummy_Handler(void)
{
    // Turn off the micro trace buffer so we don't fill it up in the infinite
    // loop below.    
    REG_MTB_MASTER = 0x00000000 + 6;

	while (1) {	}
}

/**
 * \brief Default interrupt handler for unused IRQs.
 */
void HardFault_Handler(void)
{
    // Turn off the micro trace buffer so we don't fill it up in the infinite
    // loop below.
    REG_MTB_MASTER = 0x00000000 + 6;

	while (1) {	}
}
