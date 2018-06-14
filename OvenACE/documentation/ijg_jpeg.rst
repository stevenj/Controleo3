Independent JPEG Group (IJG)
============================

The IJG (the Independent JPEG Group) JPEG decoder offers, besides JPEG decoding,
basic scaling (factors n*2) and an option to implement different memory
management backends. Refer to http://www.ijg.org for more details.

By default, the library uses malloc and free functions to manage memory.
In this case the heap size should be be set large enough for image processing.
Also in some of the functions, local arrays are allocated in stack, so the stack
size should be considered carefully, too.

Features
--------

* JPEG decoder
* JPEG encoder

Dependencies
------------

N/A

Limitations
-----------

N/A
