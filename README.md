# Controleo3 Reflow Oven Controller

## OVERVIEW

**This is the GitHub source code repository for Controleo3, as modified by Steven Johnson.**

Reflow oven build guide:
<http://www.whizoo.com/reflowoven>

Buying Controleo3:
<http://www.whizoo.com/buy>

Updating the firmware running on Controleo3:
<http://www.whizoo.com/update>

This software has been released under the GPL V3 license.

In this folder are:

1. [README.md](./README.md) - this file
2. [LICENSE](./LICENSE) - The Software License
3. A "ReflowWizard” folder containing the Reflow Wizard software

## BUILDING

This software is stand alone and requires no libraries.  Just set Arduino IDE up for ***Arduino/Genuino Zero (Native USB Port)*** and build *(verify)* ReflowWizard.ino

## HARDWARE SPECIFICATION

### Processor

The processor is an Atmel/Microchip [ATSAMD21G18](http://www.microchip.com/wwwproducts/en/ATsamd21g18)

Maximum SERCOM SPI Rate of the ATSAMD21 is 12Mhz.

### GP Outputs

Outputs 1 through 5 Control n-channel MOSFETs, which switch 4.5V (5V less schottky diode) at up to 240mA.  Terminated on Screw Terminals, active low with a common positive line.

#### GPO - Pin Assignment

| SAMD21 IO     | FUNCTION      | Note                  |
| ------------- | ------------- | --------------------- |
| PA15          | OUTPUT 1      | Arduino = D5          |
| PB30          | OUTPUT 2      | Not mapped in Arduino |
| PB17          | OUTPUT 3      | Not mapped in Arduino |
| PB09          | OUTPUT 4      | Arduino = A2          |
| PB08          | OUTPUT 5      | Arduino = A1          |

### GP IO

There is a single GPIO, directly connected to the processor.  It is on a PAD on the PCB.  Can be used as a Digital Input or Output.

#### GPIO - Pin Assignment

| SAMD21 IO     | FUNCTION      | Note          |
| ------------- | ------------- | ------------- |
| PB11          | OUTPUT 6      | Arduino = SCK |

### Servo Output

A Servo output is available.  Servo Control uses Timer Counter 3 for Timing control.  A 1-2ms pulse every 20ms.

#### SERVO - Pin Assignment

| SAMD21 IO     | FUNCTION      | Note          |
| ------------- | ------------- | ------------- |
| PB31          | SERVO OUTPUT  | Uses Timer Counter 3 (TC3) |

### Thermocouple: MAX31856

A [MAX31856](http://datasheets.maximintegrated.com/en/ds/MAX31856.pdf) thermocouple is used to read oven temperature.

#### Thermocouple - Pin Assignment

| SAMD21 IO     | Arduino IO | FUNCTION      | Note         |
| ------------- | ------------- | ------------ | ------------- |
| PA20          | 6 | MISO           | SERCOM5/3[2] |
| PA21          | 7 | MOSI           | SERCOM5/3[3] |
| PA22          | 20 | CLK          | SERCOM3/5[0] |
| PA23          | 21 | CS          | SERCOM3/5[1] |

#### Thermocouple - Notes

The MAX31856 is an SPI device, AND it is connected to the SERCOM 5/3 which can implement hardware SPI.  BUT Because it as been inexplicably wired up in such a way as to prevent the use of hardware SPI, it MUST BE BIT BASHED.  This is totally unnecessary and a waste of hardware resources.  **BAD HARDWARE DESIGN that must be corrected with software.**

### SDCard Interface

An SD Card interface is available, and is using the SDCard in SPI Mode.

#### SDCard - Pin Assignment

| SAMD21 IO     | FUNCTION      | Note         |
| ------------- | ------------ | ------------- |
| PA4          |  MISO           | SERCOM0[0] - SERCOM DI (DIPO 0x0) |
| PA5          |  CLK           | SERCOM0[1] - SERCOM CLK (DOPO 0x0/0x2) |
| PA6          |  MOSI          | SERCOM0[2] - SERCOM DO  (DOPO 0x1) |
| PA7          |  CS          | SERCOM0[3] - CS Software Controlled|

#### SDCard - Notes

The SDCard is an SPI device, AND it is connected to the SERCOM 0 which can implement hardware SPI.  BUT Because it as been inexplicably wired up in such a way as to prevent the use of hardware SPI, it MUST BE BIT BASHED.  This is totally unnecessary and a waste of hardware resources.  **BAD HARDWARE DESIGN that must be corrected with software.**

It MAY be possible to use hardware SPI for READ ONLY, but for any Write or full duplex operation the SPI must be bit bashed.

Note, the software also contains a comment seemingly bragging about a low low speed of 1.25Mhz bit bashed as some sort of accomplishment.  It is very strange.

### LCD

The board has a 480x320 LCD driven by a [ILI9488](https://www.buydisplay.com/download/ic/ILI9488.pdf) LCD Controller.

#### LCD - Pin Assignment

| SAMD21 IO | FUNCTION     | Note         |
| ----------| ------------ | ------------- |
| PB0-PB7   |  D0-D7       | 8 Bit Parallel LCD Data  |
| PB12      |  RD          | LCD READ Strobe |
| PB13      |  WR          | LCD WRITE Strobe  |
| PB14      |  D/CX        | LCD Data/Command Address |
| PB15      |  CS          | LCD Chip Select |
| PB16      |  RESET       | LCD Reset |

#### LCD - Notes

The LCD is capable of 18 bit mode, but 16 bit mode is used for performance reasons.  The hardware has no parallel bus support so the LCD Bus is done manually.

### LCD Touchscreen

A SPI LCD Touchscreen controller is connected.  The controller is a [XPT2046](https://www.buydisplay.com/download/ic/XPT2046.pdf)

#### LCD Touchscreen - Pin Assignment

| SAMD21 IO | FUNCTION     | Note         |
| ----------| ------------ | ------------- |
| PA8       |  CLK         | SERCOM0/2[0]  |
| PA9       |  CS          | SERCOM0/2[1]  |
| PA10      |  MOSI        | SERCOM0/2[2]  |
| PA11      |  MISO        | SERCOM0/2[3]  |
| PB10      |  IRQ         | |

#### LCD Touchscreen - Notes

The LCD Touchscreen is an SPI Device.  It is connected to the pins of a SERCOM which provides hardware SPI.  HOWEVER, because of a ridiculous pin assignment HARDWARE SPI can not be utilized and this device **MUST BE BIT BASHED.**  The CLK Line is on PAD0, which is not an available CLK Pin.

### Onboard Flash: W25Q80BV

The W25Q80BV(8M-bit) Serial Flash is organized into 4,096 programmable pages of 256-bytes each.  The W25Q80BV supports the standard Serial  Peripheral Interface (SPI), and a high performance Dual/Quad output as well as Dual/Quad I/O SPI.

#### FLASH - Pin Assignment

| SAMD21 IO     | FUNCTION      | Note         |
| ------------- | ------------- | ------------ |
| PA13          | CLK           | SERCOM2/4[1] |
| PA14          | CS            | SERCOM2/4[2] |
| PA16          | MOSI          | SERCOM1/3[0] |
| PA17          | MISO          | SERCOM1/3[1] |
| PA18          | WP            | SERCOM1/3[3] |
| PA19          | HOLD          | SERCOM1/3[4] |

#### FLASH - Notes

  Due to the pin assignment it is impossible to use a SERCOM to read/write the external flash and it must be bit bashed.  Quad SPI is available, which might improve performance for bit bashing.

  The current driver uses QSPI mode for this chip uses 4 bit mode for reading/writing.

## Reflow Wizard - Version Log

* 1.0  Initial public release. (21 August 2017)
* 1.1  Bug fixes and features (9 September 2017)
  * Fixed bug where servo movement would be erratic
  * Fixed bug where running learning twice could cause the oven to overheat
  * Added message when SD card wasn't FAT formatted
  * Profile commands: added "beep" and ability to specify just how much the oven door should open
* 1.2  Tweaks for ovens with slow-response heating elements (1 November 2017)
  * More responsiveness for all ovens, especially those with slow-response heating elements
  * Minor oven scoring change to better reflect reality
  * Number of reflows is now incremented as expected (Settings -> Stats)
* 1.3  Added new profile command "maintain" (15 November 2017)
  * The oven can now hold a specific temperature for a certain duration
  * See <http://www.whizoo.com/profiles> for more information
* 1.4  Minor improvement (18 December 2017)
  * When switching between non-FAT and FAT16/FAT32 formatted cards, a reboot was needed.  This has been fixed.
  * Added comments and fixed spelling mistakes in the source code

Peter Easton 2017
whizoo.com