/*
 * CDC USB Serial Comms Handler.
 *
 */
#include "atmel_asf4.h"
#include "usb_handler.h"
#include "debug_leds.h"
#include "semphr.h"
#include "printf-stdarg.h"
#include "rtos_support.h"

#define USBCDC_TX_TASK_STACK_SIZE (64)
#define USBCDC_TX_TASK_PRIORITY   (tskIDLE_PRIORITY + 10)

#define USBCDC_RX_TASK_STACK_SIZE (128)
#define USBCDC_RX_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)

static TaskHandle_t      xUSBCDC_TX_Task;
static TaskHandle_t      xUSBCDC_RX_Task;

static SemaphoreHandle_t xprintfMutex = NULL;
static bool cdc_connected = false;
static usb_cdc_control_signal_t current_control_signal_state = {0};

static uint8_t multi_desc_bytes[] = {
    /* Device descriptors and Configuration descriptors list. */
    COMPOSITE_DESCES_LS_FS};

static struct usbd_descriptors multi_desc[] = {
	{multi_desc_bytes, multi_desc_bytes + sizeof(multi_desc_bytes)}
};

/** Ctrl endpoint buffer */
static uint8_t ctrl_buffer[64];

static volatile int bulk_ins = 0;
static volatile uint8_t       in_eps[16];
static volatile enum usb_xfer_code in_rcs[16];
static volatile uint32_t      in_cnt[16];

// Buffer Size must be 256. (exploits the natural wrapping of a byte).
#define TX_BUFFER_SIZE (256)
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t tx_head = 0;
static uint8_t tx_tail = 0;
static bool    tx_idle = true;
static uint32_t tx_overrun = 0;

// Buffer Size must be 256. (exploits the natural wrapping of a byte).
#define RX_BUFFER_SIZE (256)
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_head = 0;
static uint8_t rx_tail = 0;
static uint32_t rx_overrun = 0;

// Actual Buffer we send data in over the USB. (Word aligned for DMA purposes)
static uint8_t usb_tx_buffer[CONF_USB_COMPOSITE_CDC_ACM_DATA_BULKIN_MAXPKSZ] __attribute__ ((aligned (4)));
static uint8_t usb_rx_buffer[CONF_USB_COMPOSITE_CDC_ACM_DATA_BULKIN_MAXPKSZ] __attribute__ ((aligned (4)));

static bool cdc_bulk_out(const uint8_t ep,            // The endpoint we are TXing to
                         const enum usb_xfer_code rc, // The status (should be USB_XFER_DONE)
						 const uint32_t count)        // The number of bytes we just sent.
{
	// OK, so this is how this works.
	// IF Transmit is IDLE, you call cdcdf_acm_write, with the first packet
	// of data (up to 64 bytes in size).
	// when that transfer is complete, this callback will run, where we can
	// send the next packet, until all data is sent.
	// We never directly send the packet, although we could.  Instead we
	// trigger the TX Handler Task to do it for us.

	CRITICAL_SECTION_ENTER();
	tx_idle = (tx_head == tx_tail);
	/* Notify the task that the transmission is complete. */
    vTaskNotifyGiveFromISR( xUSBCDC_TX_Task, NULL);	
	CRITICAL_SECTION_LEAVE();

	return false;                           /* No error. */
}
static bool cdcdf_demo_cb_bulk_in(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	uint32_t rx_buffered = 0;

	in_eps[bulk_ins & 0xF] = ep;
	in_rcs[bulk_ins & 0xF] = rc;
	in_cnt[bulk_ins & 0xF] = count;
	bulk_ins++;
	// On entry:
	// The last buffer set with cdcdf_acm_read will hold
	// the rx'd data.
	// ep is the endpoint we received from.
	// rc is DONE?  I am not sure if it can be anything else.
	// count is the number of valid bytes in the buffer.

	// Processing should be copy the rx data to a circular queue, and trigger the
	// task waiting on it.
	while ((rx_head+1 != rx_tail) && (rx_buffered < count)) {
		rx_buffer[rx_head++] = usb_rx_buffer[rx_buffered++];
	} 
	rx_overrun += (count - rx_buffered);
	/* Notify the task that there is something to read. */
    vTaskNotifyGiveFromISR( xUSBCDC_RX_Task, NULL);	

	// then call the cdcdf_acm_read function again with the next buffer we can fill.
	// which can be the same as the last one.
	cdcdf_acm_read(usb_rx_buffer, CONF_USB_COMPOSITE_CDC_ACM_DATA_BULKIN_MAXPKSZ); /* Another read */

	return false;                                                                                 /* No error. */
}

static bool cdcdf_demo_cb_state_c(usb_cdc_control_signal_t state)
{
	// We use the control lines to filter data.
	// RTS LOW = No Stderr messages (debugging messages).
	// DTR LOW = No stdout messages (normal operation).
	// TODO: Need to detect when connected but DTR and RTS are low.
	current_control_signal_state = state;

	if ((state.rs232.DTR) || (state.rs232.RTS)) {
		/* After connection the R/W callbacks can be registered */
		/* ACM_CB_READ is RX from the USB Serial = BULK IN */
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)cdcdf_demo_cb_bulk_in);
		/* ACM_CB_WRITE is TX to the USB Serial = BULK OUT */
		cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)cdc_bulk_out);

		/* Start Rx */
		// OK, so this call sets a RX Buffer.  
		// The next call to bulk_in callback
		// will have the data in the buffer passed.
		// This buffer should be aligned for maximum performance
		// so that DMA can be used. 
		cdcdf_acm_read(usb_rx_buffer, CONF_USB_COMPOSITE_CDC_ACM_DATA_BULKIN_MAXPKSZ);

		cdc_connected = true;
	} else {
		cdc_connected = false;
	}
	return false; /* No error. */
}

static volatile bool dont_print = false;

static void USB_CDC_TX_Handler_task(void *p)
{
	(void)p; // Unused      
	uint8_t tx_buffered;

	/* Main loop */
	while (1) {
		// Wait here until we are notified there is work to do.
		// Wait forever.
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		if (!tx_idle) {
			// Not IDLE, So MUST have data to send.
			tx_buffered = 0;
			// Copy a buffers worth from the TX queue.
			while ((tx_head != tx_tail) && 
				(tx_buffered < CONF_USB_COMPOSITE_CDC_ACM_DATA_BULKIN_MAXPKSZ)) {
				usb_tx_buffer[tx_buffered++] = tx_buffer[tx_tail++];
			} 

			cdcdf_acm_write((uint8_t*)usb_tx_buffer, tx_buffered); /* generate data */
		}
	}
}

// We are not handling serial Input in any sophisticated way.
// We just check if its a message to pass to the bootloader (HOW??)
// Or for a single key press to dump information.
static void USB_CDC_RX_Handler_task(void *p)
{
	(void)p; // Unused      
	uint8_t byte_in;

	/* Main loop */
	while (1) {
		// Wait here until we are notified there is work to do.
		// Wait forever.
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		// Process received data.
		while (rx_head != rx_tail) {
			byte_in = rx_buffer[rx_tail++];

			switch (byte_in) {
				case '?' :
					printfD("Command List:\n");
					printfD("  '?' = This Menu\n");
					printfD("  'S' = OS Statistics\n");
				break;

				case 'S' :
				case 's' :
					printfD("OS Statistics:");
					vTaskPrintRunTimeStats();
				break;

				default :
					printfD("Unknown command '%c'??\n", byte_in);
				break;
			}
		}
	}
}


void cdc_serial_comms_init(void)
{
	// The first handler, processes connection state.
	// This will register the TX/RX call backs on the first connection.
	cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)cdcdf_demo_cb_state_c);

	// Mutex used by our printf function, to prevent multiple callers.
	xprintfMutex = xSemaphoreCreateMutex();

	// Create task which handles the USB Comms
	xTaskCreate(			      
        USB_CDC_TX_Handler_task, "USBCDCTX", 
        USBCDC_TX_TASK_STACK_SIZE, NULL, 
        USBCDC_TX_TASK_PRIORITY, &xUSBCDC_TX_Task);

	xTaskCreate(			      
		USB_CDC_RX_Handler_task, "USBCDCRX",
        USBCDC_RX_TASK_STACK_SIZE, NULL, 
        USBCDC_RX_TASK_PRIORITY, &xUSBCDC_RX_Task);

}

void composite_device_init(void)
{
	/* usb stack init */
	usbdc_init(ctrl_buffer);

	cdcdf_acm_init();

	cdc_serial_comms_init();
	// Init MSC Here if needed also.
	// mscdf_init(CONF_USB_MSC_MAX_LUN);
}

void composite_device_start(void)
{
	usbdc_start(multi_desc);
	usbdc_attach();
}

void usb_init(void)
{
	composite_device_init();
	composite_device_start();

	// Create a Mass Storage handler here.		

}

uint32_t SerialTXData(bool debug, const void *buf, uint32_t cnt) {
	size_t sent = 0;

	if (cdc_connected) {
		if (!current_control_signal_state.rs232.DTR && !debug) {
			return 0; // Only write NON Debug messages when DTR asserted.
		}
		if (!current_control_signal_state.rs232.RTS && debug) {
			return 0; // Only write Debug messages when RTS asserted.
		}

		// Can ONLY call this one at a time.
		if (xSemaphoreTake( xprintfMutex, portMAX_DELAY) == pdTRUE )
		{
			// Copy a buffers worth from the TX queue.
			while (((tx_head+1) != tx_tail) && (sent < cnt)) {
				tx_buffer[tx_head++] = ((uint8_t*)buf)[sent++];
			} 

			if (tx_idle && (tx_head != tx_tail)) {
				tx_idle = false;
				xTaskNotifyGive( xUSBCDC_TX_Task );
			}

			/* We have finished accessing the shared resource.  
			Release the semaphore. */
			xSemaphoreGive( xprintfMutex );
		}	
		tx_overrun += (cnt - sent);
	}
	return sent;
}

uint32_t SerialPrint(const void *buf)
{
	uint32_t ssize = strlen(buf);
	return SerialTXData(false, buf, ssize);
}

uint32_t SerialPrintDebug(const void *buf)
{
	uint32_t ssize = strlen(buf);
	return SerialTXData(true, buf, ssize);
}


#if 0
#define MAX_PRINTF_SIZE 90
char printf_buffer[MAX_PRINTF_SIZE+1];

extern void USB_printf(const char *fmt, ...);

void USB_printf(const char *fmt, ...)
{
	int msize;
	int value;
    va_list args;
 	if (xSemaphoreTake( xprintfMutex, ( TickType_t ) 10 ) == pdTRUE )
    {
		if (cdc_connected) {
			va_start(args, fmt);
			//msize = vsnprintf(printf_buffer, MAX_PRINTF_SIZE+1, fmt, args);
			strncpy(printf_buffer,fmt,sizeof(printf_buffer));
			value = va_arg(args, int);
			printf_buffer[8] = 0x30 + ((value/10)%10);
			printf_buffer[9] = 0x30 + (value%10);

			msize = strlen(printf_buffer);
			va_end(args);

			cdcdf_acm_write((uint8_t*)printf_buffer, msize); /* generate data */
		} else if (fmt == NULL) {
			// Code path to stop my variables being optimised out.
			out_eps[bulk_outs & 0xF] = 0x55;
			out_rcs[bulk_ins & 0xF] = 0xFF;
			out_cnt[bulk_outs & 0xF] = 0xAAAAAAAA;
		}
    
    	/* We have finished accessing the shared resource.  Release the
           semaphore. */
        xSemaphoreGive( xprintfMutex );
    }	
}
#endif