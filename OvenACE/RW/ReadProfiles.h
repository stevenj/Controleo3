#ifndef __READPROFILES_H__
#define __READPROFILES_H__

#include <stdint.h>
#include "Controleo3SD.h"

// Scan the SD card, looking for profiles
void ReadProfilesFromSDCard(void);

// Look for profile files in this directory
void processDirectory(File dir);

// Process a file with a TXT extension
void processFile(File file);

// Set the tokens back to the beginning to continue the search for them
void initTokenPtrs(void);

// If the given character matches the required character in the token then advance
// the token search.  Return the found token if this was the last character
uint8_t hasTokenBeenFound(char c);

// Read a string from the file.  The string must be contained inside double-quotes.
// Return false if the end-of-file is reached before the second double-quote is read.
// Only save up to the maximum string length, and ignore (discard) any characters over
// the maximum length
bool getStringFromFile(File file, char *strBuffer, uint8_t maxLength);

// Read a number from the file.  This method doesn't care what the delimiter is; it just 
// reads until it finds a digit, and continues until it finds something that isn't a
// digit.  This reads uint16_t numbers, so they are limited to 65,536 (2^16).
// Return false if the end-of-file is reached before the number is found.
bool getNumberFromFile(File file, uint16_t *num);

// Convert the token to readable text
char *tokenToText(char *str, uint8_t token, uint16_t *numbers);

// Initialize all the variables needed to write the profile to flash
void initProfileWriteToFlash(uint16_t startingBlock);

// Save the token and its parameters to flash
void saveTokenAndNumbersToFlash(uint8_t token, uint16_t *numbers, uint8_t numOfNumbers);

// Save a string to flash.  Currently, only "Display" takes a string as
// a parameter.  The string is saved null-terminated.
void saveTokenAndStringToFlash(uint16_t token, char *str);

// Write the 256-byte profile block to flash
bool writeTokenBufferToFlash(bool endOfProfile);

// Get the next token from flash.  If str is smaller than 512 then it is the first
// block number and the flash reading must be initialized to that block.  It's
// a bit hacky, but means we can use reduced-scope static variables.
uint16_t getNextTokenFromFlash(char *str, uint16_t *num);

// Dump profile for debugging
void dumpProfile(uint8_t profileNo);

// Return true if a profile of the same name exists
bool profileExists(char *profileName);

// Delete profiles having the same name so you don't get duplicates
void deleteProfileByName(char *profileName);

// Find the first flash block available to store a profile.  Profiles are stored in
// sequential blocks to avoid flash wear
uint16_t getFreeProfileBlock(void);

// Keep profiles in alphabetical order, with unused profiles at the end
// Yes, inefficient and slow.  But how often is this called?  Only when
// profiles are read from the SD card (and the SD card is really slow!)
void sortProfiles(void);

// Delete the selected profile
// Erase the profile block so that it can be used again, delete
// the profile then sort the remaining ones.
void deleteProfile(uint8_t num);

// Get the number of profiles.
uint8_t getNumberOfProfiles(void);

#endif