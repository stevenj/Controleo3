/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */
#include "atmel_asf4.h"
#include "usb_handler.h"

/* Max LUN number */
#define CONF_USB_MSC_MAX_LUN 0

/* MSC LUN number */
#define LUN_RAM 0

/* MSC LUN definitions */
#define DISK_INFORMATION(n)                                                                                            \
	{                                                                                                                  \
		CONF_USB_MSC_LUN##n##_TYPE, (CONF_USB_MSC_LUN##n##_RMB << 7),                                                  \
		    ((CONF_USB_MSC_LUN##n##_ISO << 6) + (CONF_USB_MSC_LUN##n##_ECMA << 3) + CONF_USB_MSC_LUN##n##_ANSI),       \
		    CONF_USB_MSC_LUN##n##_REPO, 31, 0x00, 0x00, 0x00, CONF_USB_MSC_LUN##n##_FACTORY,                           \
		    CONF_USB_MSC_LUN##n##_PRODUCT, CONF_USB_MSC_LUN##n##_PRODUCT_VERSION                                       \
	}

#define DISK_CAPACITY(n)                                                                                               \
	{                                                                                                                  \
		(uint8_t)(CONF_USB_MSC_LUN##n##_LAST_BLOCK_ADDR >> 24),                                                        \
		    (uint8_t)(CONF_USB_MSC_LUN##n##_LAST_BLOCK_ADDR >> 16),                                                    \
		    (uint8_t)(CONF_USB_MSC_LUN##n##_LAST_BLOCK_ADDR >> 8),                                                     \
		    (uint8_t)(CONF_USB_MSC_LUN##n##_LAST_BLOCK_ADDR >> 0),                                                     \
		    (uint8_t)((uint32_t)(CONF_USB_MSC_LUN##n##_BLOCK_SIZE) >> 24),                                             \
		    (uint8_t)((uint32_t)(CONF_USB_MSC_LUN##n##_BLOCK_SIZE) >> 16),                                             \
		    (uint8_t)((uint32_t)(CONF_USB_MSC_LUN##n##_BLOCK_SIZE) >> 8),                                              \
		    (uint8_t)((uint32_t)(CONF_USB_MSC_LUN##n##_BLOCK_SIZE) >> 0)                                               \
	}

/* MSC LUN settings */

static uint8_t *ram_disk_buf;

static uint8_t       msc_lun;
static bool          xfer_dir;
static volatile bool usb_busy;
static uint32_t      disk_addr;
static uint32_t      usb_remain = 0;

/* Inquiry Information */
static uint8_t inquiry_info[CONF_USB_MSC_MAX_LUN + 1][36] = {DISK_INFORMATION(0)};

/* Capacities of Disk */
static uint8_t format_capa[CONF_USB_MSC_MAX_LUN + 1][8] = {DISK_CAPACITY(0)};

#if CONF_USBD_HS_SP
static uint8_t multi_desc_bytes[] = {
    /* Device descriptors and Configuration descriptors list. */
    COMPOSITE_HS_DESCES_LS_FS};
static uint8_t multi_desc_bytes_hs[] = {
    /* Device descriptors and Configuration descriptors list. */
    COMPOSITE_HS_DESCES_HS};
#else
static uint8_t multi_desc_bytes[] = {
    /* Device descriptors and Configuration descriptors list. */
    COMPOSITE_DESCES_LS_FS};
#endif

static struct usbd_descriptors multi_desc[] = {{multi_desc_bytes, multi_desc_bytes + sizeof(multi_desc_bytes)}
#if CONF_USBD_HS_SP
                                               ,
                                               {multi_desc_bytes_hs, multi_desc_bytes_hs + sizeof(multi_desc_bytes_hs)}
#endif
};

/** Ctrl endpoint buffer */
static uint8_t ctrl_buffer[64];

#if CONF_USB_COMPOSITE_CDC_ECHO_DEMO
static uint8_t *cdcdf_demo_buf;
static bool cdcdf_demo_cb_bulk_out(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	cdcdf_acm_write(cdcdf_demo_buf, count); /* Echo data */
	return false;                           /* No error. */
}
static bool cdcdf_demo_cb_bulk_in(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	cdcdf_acm_read((uint8_t *)cdcdf_demo_buf, CONF_USB_COMPOSITE_CDC_ACM_DATA_BULKIN_MAXPKSZ_HS); /* Another read */
	return false;                                                                                 /* No error. */
}
static bool cdcdf_demo_cb_state_c(usb_cdc_control_signal_t state)
{
	if (state.rs232.DTR) {
		/* After connection the R/W callbacks can be registered */
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)cdcdf_demo_cb_bulk_out);
		cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)cdcdf_demo_cb_bulk_in);
		/* Start Rx */
		cdcdf_acm_read((uint8_t *)cdcdf_demo_buf, CONF_USB_COMPOSITE_CDC_ACM_DATA_BULKIN_MAXPKSZ_HS);
	}
	return false; /* No error. */
}
void cdcdf_acm_demo_init(uint8_t *bulk_packet_buffer)
{
	cdcdf_demo_buf = bulk_packet_buffer;
	cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)cdcdf_demo_cb_state_c);
}
#endif /* CONF_USB_COMPOSITE_CDC_ECHO_DEMO */

/**
 * \brief Eject Disk
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static int32_t msc_disk_eject(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return ERR_NOT_FOUND;
	}
	return ERR_NONE;
}

/**
 * \brief Inquiry whether Disk is ready
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static int32_t msc_disk_is_ready(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return ERR_NOT_FOUND;
	}
	return ERR_NONE;
}

/**
 * \brief Callback invoked when a new read blocks command received
 * \param[in] lun logic unit number
 * \param[in] addr start address of disk to be read
 * \param[in] nblocks block amount to be read
 * \return Operation status.
 */
static int32_t msc_new_read(uint8_t lun, uint32_t addr, uint32_t nblocks)
{
	int32_t  rc  = msc_disk_is_ready(lun);
	uint32_t lba = 0;
	if (rc != ERR_NONE) {
		return rc;
	}
	lba = (format_capa[lun][0] << 24) + (format_capa[lun][1] << 16) + (format_capa[lun][2] << 8) + format_capa[lun][3];
	if (addr > lba || addr + nblocks > lba + 1) {
		return ERR_BAD_ADDRESS;
	}
	msc_lun  = lun;
	xfer_dir = true;

	disk_addr  = addr;
	usb_remain = nblocks;

	return ERR_NONE;
}

/**
 * \brief Callback invoked when a new write blocks command received
 * \param[in] lun logic unit number
 * \param[in] addr start address of disk to be written
 * \param[in] nblocks block amount to be written
 * \return Operation status.
 */
static int32_t msc_new_write(uint8_t lun, uint32_t addr, uint32_t nblocks)
{
	int32_t  rc  = msc_disk_is_ready(lun);
	uint32_t lba = 0;
	if (rc != ERR_NONE) {
		return rc;
	}
	lba = (format_capa[lun][0] << 24) + (format_capa[lun][1] << 16) + (format_capa[lun][2] << 8) + format_capa[lun][3];
	if (addr > lba || addr + nblocks > lba + 1) {
		return ERR_BAD_ADDRESS;
	}
	msc_lun  = lun;
	xfer_dir = false;

	disk_addr  = addr;
	usb_remain = nblocks;

	return ERR_NONE;
}

/**
 * \brief Callback invoked when a blocks transfer is done
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static int32_t msc_xfer_done(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return ERR_DENIED;
	}
	usb_busy = false;
	if (LUN_RAM == msc_lun) {
		usb_remain = 0;
		if (!xfer_dir) {
			/* Terminate write */
			mscdf_xfer_blocks(false, NULL, 0);
		}
		return ERR_NONE;
	}
	return ERR_NONE;
} /**
  * \brief Disk loop
  */
static void msc_disk_task(void)
{
	/* RAM Disk transfer directly between RAM and USB */
}

/**
 * \brief USB loop
 */
static void msc_usb_task(void)
{
	if (false == usb_busy && 0 != usb_remain) {
		if (0 == xfer_dir) {
			/* RAM Disk directly exchange data with USB */
			if (msc_lun == LUN_RAM) {
				usb_busy = true;
				mscdf_xfer_blocks(false, &ram_disk_buf[disk_addr << 9], usb_remain);
				return;
			}
		} else {
			/* RAM Disk directly exchange data with USB */
			if (msc_lun == LUN_RAM) {
				usb_busy = true;
				mscdf_xfer_blocks(true, &ram_disk_buf[disk_addr << 9], usb_remain);
				return;
			}
		}
	}
}

/**
 * \brief Callback invoked when inquiry data command received
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static uint8_t *msc_inquiry_info(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return NULL;
	} else {
		usb_busy   = false;
		usb_remain = 0;
		return &inquiry_info[lun][0];
	}
}

/**
 * \brief Callback invoked when read format capacities command received
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static uint8_t *msc_get_capacity(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return NULL;
	} else {
		return &format_capa[lun][0];
	}
}

void mscdf_demo_init(uint8_t *ramd_buf, uint8_t *usbdisk_buf)
{
	ram_disk_buf = ramd_buf;
	mscdf_register_callback(MSCDF_CB_INQUIRY_DISK, (FUNC_PTR)msc_inquiry_info);
	mscdf_register_callback(MSCDF_CB_GET_DISK_CAPACITY, (FUNC_PTR)msc_get_capacity);
	mscdf_register_callback(MSCDF_CB_START_READ_DISK, (FUNC_PTR)msc_new_read);
	mscdf_register_callback(MSCDF_CB_START_WRITE_DISK, (FUNC_PTR)msc_new_write);
	mscdf_register_callback(MSCDF_CB_EJECT_DISK, (FUNC_PTR)msc_disk_eject);
	mscdf_register_callback(MSCDF_CB_TEST_DISK_READY, (FUNC_PTR)msc_disk_is_ready);
	mscdf_register_callback(MSCDF_CB_XFER_BLOCKS_DONE, (FUNC_PTR)msc_xfer_done);
}

void mscdf_demo_task(void)
{
	msc_disk_task();
	msc_usb_task();
}

void composite_device_init(void)
{
	/* usb stack init */
	usbdc_init(ctrl_buffer);

/* usbdc_register_funcion inside */
	cdcdf_acm_init();
	mscdf_init(CONF_USB_MSC_MAX_LUN);
}

void composite_device_start(void)
{
	usbdc_start(multi_desc);
	usbdc_attach();
}

static void USB_Handler_task(void *p)
{
	(void)p; // Unused

	/* Initialize */
	/* It's done with system init ... */

	/* Before start do function related initializations */
	/* Add your code here ... */

	/* Start device */
	composite_device_start();

	/* Main loop */
	while (1) {
		if (cdcdf_acm_is_enabled()) {
			/* CDC ACM process*/
		}
		if (mscdf_is_enabled()) {
			/* MSC process */
		}
	}
}

#define USBTASK_STACK_SIZE (256 / sizeof(portSTACK_TYPE))
#define USBTASK_PRIORITY   (tskIDLE_PRIORITY + 1)

static TaskHandle_t      xUSBTask;


void usb_init(void)
{
	composite_device_init();

	// Create task which handles the USB Comms
	xTaskCreate(
        USB_Handler_task, "USB Handler", 
        USBTASK_STACK_SIZE, NULL, 
        USBTASK_PRIORITY, xUSBTask);

}
