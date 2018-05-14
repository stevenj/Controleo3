Controleo3 Reflow Oven Controller
=================================

Reflow oven build guide:  
http://www.whizoo.com/reflowoven

Buying Controleo3:  
http://www.whizoo.com/buy

Updating the firmware running on Controleo3:  
http://www.whizoo.com/update

### This is the GitHub source code repository for Controleo3.

This software has been released under the CC BY-NC-SA 3.0 license.  You are free
To use it any way you wish.

In this folder are:
1. README - this file
2. Files that comprise the Controleo3 library for Arduino
3. An "examples” folder containing the Reflow Wizard software

To install the Controleo3 library, please refer to:  
http://arduino.cc/en/Guide/Libraries or http://www.whizoo.com/update

Hardware Specification
======================

## Onboard Flash: W25Q80BV
The W25Q80BV(8M-bit) Serial Flash is organized into 4,096 programmable pages of 256-bytes each.  The W25Q80BV supports the standard Serial  Peripheral Interface (SPI), and a high performance Dual/Quad output as well as Dual/Quad I/O SPI.

### Pin Assignment
| SAMD21 IO     | FUNCTION    | Note |
| ------------- |-------------|------|
| PA13          | CLK         | SERCOM2/4[1] |
| PA14          | CS          | SERCOM2/4[2] |
| PA16          | MOSI        | SERCOM1/3[0] |
| PA17          | MISO        | SERCOM1/3[1] |
| PA18          | WP          | SERCOM1/3[3] |
| PA19          | HOLD        | SERCOM1/3[4] |

### Notes
  Due to the pin assignment it is impossible to use a SERCOM to read/write the external flash and it must be bit bashed.  Quad SPI is available, which might improve performance for bit bashing.

  The current driver uses QSPI mode for this chip uses 4 bit mode for reading/writing.

Reflow Wizard
=============
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
  * See http://www.whizoo.com/profiles for more information
* 1.4  Minor improvement (18 December 2017)
  * When switching between non-FAT and FAT16/FAT32 formatted cards, a reboot was needed.  This has been fixed.
  * Added comments and fixed spelling mistakes in the source code

Peter Easton 2017  
whizoo.com


