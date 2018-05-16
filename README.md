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