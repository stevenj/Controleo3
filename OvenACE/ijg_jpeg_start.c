/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */

#include "atmel_start.h"
#include "ijg_jpeg_start.h"

/* 320 x 240 Jpeg in jpegdata */
#include "jpegfile.h"

#include <setjmp.h>

/* ---------IJG JPEG library callback functions -------------------*/

#include <jerror.h>

/* We use C's setjmp/longjmp facility to return control.  This means that the
* routine which calls the JPEG library must first execute a setjmp() call to
* establish the return point.  We want the replacement error_exit to do a
* longjmp().  But we need to make the setjmp buffer accessible to the
* error_exit routine.  To do this, we make a private extension of the
* standard JPEG error handler object.  (If we were using C++, we'd say we
* were making a subclass of the regular error handler.)
*
* Here's the extended error handler struct:
*/

struct my_error_mgr {
	struct jpeg_error_mgr pub; /* "public" fields */

	jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

/*
 * Here's the routine that will replace the standard error_ methods,
 * so that the _read/_write will not be linked due to STDIO introduction.
 */

METHODDEF(void)
my_reset_error_mgr(j_common_ptr cinfo)
{
	cinfo->err->num_warnings = 0;
	/* trace_level is not reset since it is an application-supplied parameter */
	cinfo->err->msg_code = 0; /* may be useful as a flag for "no error" */
}

METHODDEF(void)
my_emit_message(j_common_ptr cinfo, int msg_level)
{
}

METHODDEF(void)
my_output_message(j_common_ptr cinfo)
{
}

METHODDEF(void)
my_format_message(j_common_ptr cinfo, char *buffer)
{
}

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr extended_err = (my_error_ptr)cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message)(cinfo);

	/* Return control to the setjmp point */
	longjmp(extended_err->setjmp_buffer, 1);
}

/* This struct contains the JPEG decompression parameters and pointers to
 * working space (which is allocated as needed by the JPEG library).
 */
static struct jpeg_decompress_struct cinfo;
/* We use our private extension JPEG error handler.
 * Note that this struct must live as long as the main JPEG parameter
 * struct, to avoid dangling-pointer problems.
 */
static struct my_error_mgr jerr;

/* JSAMPLEs per row in output buffer */
static JSAMPLE  row[320];
static JSAMPROW row_array[] = {row};
/* Make a one-row-high sample array that will go away when done with image */
static JSAMPARRAY buffer = {row_array};

/*
 * Modified example from IJG JPEG lib for JPEG decompression.
 * Note that since the lib has stack and heap size requirements,
 * the stack and heap size should be considered carefully to avoid memory
 * management problem.
 */
void JPEG_CODEC_0_example(void)
{
	/* Step 1: allocate and initialize JPEG decompression object */

	/* We customize the error-handling methods in a jpeg_error_mgr object
	 * so that the errors are not output, to avoid links to STDIO objects.
	 */
	jerr.pub.error_exit          = my_error_exit;
	jerr.pub.emit_message        = my_emit_message;
	jerr.pub.output_message      = my_output_message;
	jerr.pub.format_message      = my_format_message;
	jerr.pub.reset_error_mgr     = my_reset_error_mgr;
	jerr.pub.trace_level         = 0; /* default = no tracing */
	jerr.pub.num_warnings        = 0; /* no warnings emitted yet */
	jerr.pub.msg_code            = 0; /* may be useful as a flag for "no error" */
	jerr.pub.jpeg_message_table  = NULL;
	jerr.pub.last_jpeg_message   = 0;
	jerr.pub.addon_message_table = NULL;
	jerr.pub.first_addon_message = 0; /* for safety */
	jerr.pub.last_addon_message  = 0;

	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		return;
	}
	cinfo.err = &jerr.pub;

	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */
	jpeg_mem_src(&cinfo, jpegdata, sizeof(jpegdata));

	/* Step 3: read file parameters with jpeg_read_header() */
	(void)jpeg_read_header(&cinfo, TRUE);

	/* Step 4: set parameters for decompression */
	cinfo.dct_method          = JDCT_FASTEST;
	cinfo.do_fancy_upsampling = false;
	cinfo.out_color_space     = JCS_GRAYSCALE;

	/* Step 5: Start decompresser */
	(void)jpeg_start_decompress(&cinfo);
	/* We may need to do some setup of our own at this point before reading
	 * the data.  After jpeg_start_decompress() we have the correct scaled
	 * output image dimensions available, as well as the output colormap
	 * if we asked for color quantization.
	 * In this example, we need to make an output work buffer of the right size.
	 */
	/* JSAMPARRAY are defined static in the example to
	 * minimize the use of STACK and HEAP. */

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	while (cinfo.output_scanline < cinfo.output_height) {
		/* get decoded scanlines */
		jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Handle the decoded scanlines in buffer */
	}

	/* Step 7: Finish decompression */
	jpeg_finish_decompress(&cinfo);

	/* Step 8: Release JPEG decompression object */
	jpeg_destroy_decompress(&cinfo);
}
