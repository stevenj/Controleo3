// Written by Peter Easton
// Released under CC BY-NC-SA 3.0 license
// Build a reflow oven: http://whizoo.com
//
#include "ReadProfiles.h"
#include "ReflowWizard.h"
#include "Render.h"
#include "Prefs.h"
#include "printf-stdarg.h"
#include "rtos_support.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "ArduinoDefs.h"

uint8_t flashBuffer256Bytes[256];     // Read/write from flash.  This is the size of a flash block

// Read all the profiles from the SD card.  The profiles can be in sub-directories

// This is the instruction set.  The instructions must be unique
char *tokenString[NUM_TOKENS] = {(char *) "not_a_token", (char *) "name", (char *) "#", (char *) "//", (char *) "deviation", (char *) "maximum temperature",
                                 (char *) "initialize timer", (char *) "start timer", (char *) "stop timer", (char *) "maximum duty", (char *) "display",
                                 (char *) "open door", (char *) "close door", (char *) "bias", (char *) "convection fan on", (char *) "convection fan off",
                                 (char *) "cooling fan on", (char *) "cooling fan off", (char *) "ramp temperature", (char *) "element duty cycle",
                                 (char *) "wait for", (char *) "wait until above", (char *) "wait until below", (char *) "play tune", (char *) "play beep",
                                 (char *) "door percentage", (char *) "maintain"};
char *tokenPtr[NUM_TOKENS];

// Scan the SD card, looking for profiles
void ReadProfilesFromSDCard()
{
  // Don't do anything if there isn't a SD card
  if (digitalRead(A0) == 1)
    return;
  printfD("SD card is present\n");

  // Initialize the SD card
  printfD("Initializing SD card...\n");
  // Try initializing twice.  Necessary if good card follows bad one
  if (!SD.begin() && !SD.begin()) {
    printfD((char *) "Card failed, or not present\n");
    printfD((char *) "Error! Is SD card FAT16 or FAT32?\n");
    tft.fillRect(20, 120, 440, 40, WHITE);
    displayString(24, 120, FONT_9PT_BLACK_ON_WHITE, (char *) "Error! Is SD card FAT16 or FAT32?\n");
    uint32_t start = millis();
    // Display the message for 3 seconds, or until the SD card is removed
    while (digitalRead(A0) == 0 && millis() - start < 3000)
      delay(20);
    tft.fillRect(20, 120, 440, 40, WHITE);
    // Don't do anything more
    return;
  }
  printfD("SD Card initialized\n");

  // Open the root folder to look for files
  File root = SD.open("/");

  processDirectory(root);

  // Profiles are written to flash as part of the factory setup.  Write these immediately
  if (prefs.sequenceNumber < 10)
    writePrefsToFlash();
}


// Look for profile files in this directory
void processDirectory(File dir)
{
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      // No more files.  Close the directory now
      dir.close();
      return;
    }

    // If this is a directory then process it (uses recursion)
    if (entry.isDirectory())
      processDirectory(entry);
    else {
      // Only look at TXT files
      if (strstr(entry.name(), ".TXT"))
        processFile(entry);
    }
    entry.close();
  }
}


// Process a file with a TXT extension
void processFile(File file)
{
  profiles *newProfile = 0;
  char c;
  uint16_t numbers[4];  // Array used to store numbers read from the file
  uint8_t token;
  
  // Some sanity checks on the file before processing it
  if (file.size() < 100)
    return;
  // The file must start with "Controleo3"
  if (file.read(buffer100Bytes, 10) != 10)
    return;
  buffer100Bytes[10] = 0;
  if (strcmp(buffer100Bytes, "Controleo3") != 0)
    return;

  // Looks like this is a valid profile file
  printfD("Processing file: %s\n", file.name());

  // Reset the token search
  initTokenPtrs();

  // Keep reading characters until the entire file has been processed
  while (file.available()) {
    // See if this character resulted in a token being found
    token = hasTokenBeenFound(file.read());
    if (token == NOT_A_TOKEN)
        continue;

    // A token was found!
    // Reset the token pointers now (before we forget)
    initTokenPtrs();

    switch (token) {
      case TOKEN_NAME:
        // Has a name been extracted from the file already?
        if (newProfile) {
          printfD("Profile has more than one name!\n");
          goto tokenError;
        }
        // Get the name of the profile
        if (!getStringFromFile(file, buffer100Bytes, MAX_PROFILE_NAME_LENGTH)) {
          printfD("Unable to find profile name\n");
          goto tokenError;
        }

        // If this profile name exists then it is probably being updated.  Delete it so the new one can be saved
        deleteProfileByName(buffer100Bytes);

        // Is there a spare profile slot?
        if (prefs.numProfiles >= MAX_PROFILES) {
          printfD("No space to store profile\n");
          goto tokenError;
        }

        // Point to the first available space that this profile will be saved to
        newProfile = &prefs.profile[prefs.numProfiles];

        // Save the name
        strcpy(newProfile->name, buffer100Bytes);
        
        // Allocate the first flash block to this profile
        newProfile->startBlock = getFreeProfileBlock();
        if (!newProfile->startBlock)
          goto tokenError;

        // The flash block should be erased already - but make sure
        flash.eraseProfileBlock(newProfile->startBlock);
        
        prefs.lastUsedProfileBlock = newProfile->startBlock;
        initProfileWriteToFlash(newProfile->startBlock);

        newProfile->noOfTokens = 0;
        newProfile->peakTemperature = 0;
        break;

      case TOKEN_COMMENT1:
      case TOKEN_COMMENT2:
        // Discard everything until a new line character
        while (file.available()) {
          c = file.read();
          if (c == 0x0A || c == 0x0D)
            break;
        }
        break;

      case TOKEN_DISPLAY:
        // This should be followed by a string that should be displayed
        if (!getStringFromFile(file, buffer100Bytes, MAX_PROFILE_DISPLAY_STR)) {
          printfD("Error getting display string\n");
          goto tokenError;
        }
        // Save the display string
        saveTokenAndStringToFlash(token, buffer100Bytes);
        newProfile->noOfTokens++;
        break;

      case TOKEN_MAX_DUTY:
      case TOKEN_ELEMENT_DUTY_CYCLES:
      case TOKEN_BIAS:
        // This should be followed by 3 numbers, indicating bottom/top/boost
        if (!getNumberFromFile(file, &numbers[0])) {
          printfD("Error getting number 1/3\n");
          goto tokenError;
        }
        if (!getNumberFromFile(file, &numbers[1])) {
          printfD("Error getting number 2/3\n");
          goto tokenError;
        }
        if (!getNumberFromFile(file, &numbers[2])) {
          printfD("Error getting number 3/3\n");
          goto tokenError;
        }
        // Save the token and numbers to flash
        saveTokenAndNumbersToFlash(token, numbers, 3);
        newProfile->noOfTokens++;
        break;

      case TOKEN_TEMPERATURE_TARGET:
      case TOKEN_OVEN_DOOR_PERCENT:
      case TOKEN_MAINTAIN_TEMP:
        // These should be followed by 2 numbers
        if (!getNumberFromFile(file, &numbers[0])) {
          printfD("Error getting number 1/2\n");
          goto tokenError;
        }
        if (!getNumberFromFile(file, &numbers[1])) {
          printfD("Error getting number 2/2\n");
          goto tokenError;
        }
        // Save the token and numbers to flash
        saveTokenAndNumbersToFlash(token, numbers, 2);
        newProfile->noOfTokens++;
        // This could be the peak temperature
        if (numbers[0] > newProfile->peakTemperature)
          newProfile->peakTemperature = numbers[0];
        break;
        
      case TOKEN_DEVIATION:
      case TOKEN_MAX_TEMPERATURE:
      case TOKEN_INITIALIZE_TIMER:
      case TOKEN_OVEN_DOOR_OPEN:
      case TOKEN_OVEN_DOOR_CLOSE:
      case TOKEN_WAIT_FOR_SECONDS:
      case TOKEN_WAIT_UNTIL_ABOVE_C:
      case TOKEN_WAIT_UNTIL_BELOW_C:
        // These require 1 parameter
        if (!getNumberFromFile(file, &numbers[0])) {
          printfD("Error getting number\n");
          goto tokenError;
        }
        // Save the oven open/close to flash
        saveTokenAndNumbersToFlash(token, numbers, 1);
        newProfile->noOfTokens++;
        // This could be the peak temperature
        if (token == TOKEN_WAIT_UNTIL_ABOVE_C && numbers[0] > newProfile->peakTemperature)
          newProfile->peakTemperature = numbers[0];
        break;

      case TOKEN_START_TIMER:
      case TOKEN_STOP_TIMER:
      case TOKEN_CONVECTION_FAN_ON:
      case TOKEN_CONVECTION_FAN_OFF:
      case TOKEN_COOLING_FAN_ON:
      case TOKEN_COOLING_FAN_OFF:
      case TOKEN_PLAY_DONE_TUNE:
      case TOKEN_PLAY_BEEP:
        // Just save the token to flash.  These don't take parameters
        saveTokenAndNumbersToFlash(token, numbers, 0);
        newProfile->noOfTokens++;
        break;
    } 
  }

  // Done reading the file
  // Save the tokens in the profile to flash.  There might be an error if there were too many tokens in the file
  if (!writeTokenBufferToFlash(true))
    goto tokenError;
    
  // Sort the profiles to keep them in alphabetical order.  This also updates prefs.numProfiles
  sortProfiles();
  // Save the preferences
  savePrefs();
  return;

tokenError:
  // If there was any error, throw the entire thing away.  Better that the user see that the profile
  // wasn't read than it was read - but not knowing if it was read correctly or not.
  // Unfortunately this doesn't take into account incorrectly spelt or ordered tokens (e.g. "door close" instead of "close door")
  
  // Delete this profile
  deleteProfile(prefs.numProfiles);

  // Save the profiles
  savePrefs();
  printfD("Error processing file - discarded\n");
}


// Set the tokens back to the beginning to continue the search for them
void initTokenPtrs()
{
  for (uint8_t i=0; i < NUM_TOKENS; i++)
    tokenPtr[i] = tokenString[i];
}


// If the given character matches the required character in the token then advance
// the token search.  Return the found token if this was the last character
uint8_t hasTokenBeenFound(char c)
{
  // Make the token search case-insensitive
  c = tolower(c);
  // Start this loop at 1 because the first "token" is not-a-token
  for (uint8_t i=1; i < NUM_TOKENS; i++) {
    // Does this character match?
    if (c == *tokenPtr[i]) {
      // Advance the token pointer
      tokenPtr[i]++;
      // Is this the end of the string?
      if (*tokenPtr[i] == 0) {
        return i;
      }
    }
    else {
      // The character didn't match.  Reset the token pointer
      tokenPtr[i] = tokenString[i];
    }
  }
  // This wasn't the last character of any of the tokens
  return NOT_A_TOKEN;
}


// Read a string from the file.  The string must be contained inside double-quotes.
// Return false if the end-of-file is reached before the second double-quote is read.
// Only save up to the maximum string length, and ignore (discard) any characters over
// the maximum length
bool getStringFromFile(File file, char *strBuffer, uint8_t maxLength)
{
  bool doubleQuoteFound = false;
  char c;

  // Empty string so far
  memset(strBuffer, 0, 20);
  
  while (file.available()) {
    c = file.read();
    // Is this the first double-quote (the start of the string)?
    if (!doubleQuoteFound) {
      if (c == '"') 
        doubleQuoteFound = true;
      continue;
    }

    // Is this the second double-quote (the end of the string)?
    if (doubleQuoteFound && c == '"') {
      // Terminate the string
      *strBuffer = 0;
      return true;
    }

    // Have we come to the end of the line without finding a double-quote?
    if (c == 0x0A || c == 0x0D) {
      printfD("Missing double-quote of string\n");
      return false;
    }
    
    // Save this character if the max length hasn't been exceeded
    if (maxLength) {
      *strBuffer++ = c;
      maxLength--;
    }
  }
  // We've reached the end of the file
  return false;
}


// Read a number from the file.  This method doesn't care what the delimiter is; it just 
// reads until it finds a digit, and continues until it finds something that isn't a
// digit.  This reads uint16_t numbers, so they are limited to 65,536 (2^16).
// Return false if the end-of-file is reached before the number is found.
bool getNumberFromFile(File file, uint16_t *num)
{
  bool digitFound = false;
  char c;

  *num = 0;
  
  while (file.available()) {
    c = file.read();
      
    if (!digitFound) {
      // Have we come to the end of the line without finding a digit?
      if (c == 0x0A || c == 0x0D) {
        printfD("Can't find number\n");
        return false;
      }

      // Is this the first digit?
      if (isdigit(c)) {
        digitFound = true;
        *num = c - '0';
        continue;
      }

      // This isn't a digit, and we haven't found one yet.  Keep looking
      continue;
    }

    // We have found a number already.  Is this the delimiter?
    if (!isdigit(c))
      return true;

    // We have found another digit of the number
    *num = (*num * 10) + c - '0';
  }
  
  // We've reached the end of the file
  if (digitFound) {
      printfD("Found number %d\n", *num);
      return true;
  }
  return false;
}


// Convert the token to readable text
char *tokenToText(char *str, uint8_t token, uint16_t *numbers)
{
  *str = 0;
  switch (token) {
    case TOKEN_DEVIATION:
      sprintf(str, "Deviation to abort %dC", numbers[0]);
      break;
    case TOKEN_MAX_TEMPERATURE:
      sprintf(str, "Maximum temperature %dC", numbers[0]);
      break;
    case TOKEN_INITIALIZE_TIMER:
      sprintf(str, "Initialize timer to %d seconds", numbers[0]);
      break;
    case TOKEN_START_TIMER:
      strcpy(str, "Start timer");
      break;
    case TOKEN_STOP_TIMER:
      strcpy(str, "Stop timer");
      break;
    case TOKEN_MAX_DUTY:
      sprintf(str, "Maximum duty %d/%d/%d", numbers[0], numbers[1], numbers[2]);
      break;
    case TOKEN_OVEN_DOOR_OPEN:
      sprintf(str, "Open door over %d seconds", numbers[0]);
      break;
    case TOKEN_OVEN_DOOR_CLOSE:
      sprintf(str, "Close door over %d seconds", numbers[0]);
      break;
    case TOKEN_OVEN_DOOR_PERCENT:
      sprintf(str, "Door percentage %d%% over %d seconds", numbers[0], numbers[1]);
      break;
    case TOKEN_BIAS:
      sprintf(str, "Bias %d/%d/%d", numbers[0], numbers[1], numbers[2]);
      break;
    case TOKEN_CONVECTION_FAN_ON:
      strcpy(str, "Convection fan on");
      break;
    case TOKEN_CONVECTION_FAN_OFF:
      strcpy(str, "Convection fan off");
      break;
    case TOKEN_COOLING_FAN_ON:
      strcpy(str, "Cooling fan on");
      break;
    case TOKEN_COOLING_FAN_OFF:
      strcpy(str, "Cooling fan off");
      break;
    case TOKEN_TEMPERATURE_TARGET:
      sprintf(str, "Ramp temperature to %dC in %d seconds", numbers[0], numbers[1]);
      break;
    case TOKEN_MAINTAIN_TEMP:
      sprintf(str, "Maintain %dC for %d seconds", numbers[0], numbers[1]);
      break;
    case TOKEN_ELEMENT_DUTY_CYCLES:
      sprintf(str, "Element duty cycle %d/%d/%d", numbers[0], numbers[1], numbers[2]);
      break;
    case TOKEN_WAIT_FOR_SECONDS:
      sprintf(str, "Wait for %d seconds", numbers[0]);
      break;
    case TOKEN_WAIT_UNTIL_ABOVE_C:
      sprintf(str, "Wait until above %dC", numbers[0]);
      break;
    case TOKEN_WAIT_UNTIL_BELOW_C:
      sprintf(str, "Wait until below %dC", numbers[0]);
      break;
    case TOKEN_PLAY_DONE_TUNE:
      strcpy(str, "Play tune");
      break;
    case TOKEN_PLAY_BEEP:
      strcpy(str, "Play beep");
      break;
  }
  return str;  
}


uint16_t startFlashBlock;
uint16_t offsetIntoBlock;
uint16_t numBlocksUsed;

// Initialize all the variables needed to write the profile to flash
void initProfileWriteToFlash(uint16_t startingBlock) {
  startFlashBlock = startingBlock;
  offsetIntoBlock = 0;
  numBlocksUsed = 0;

  // Clear the buffer used to save the profile
  memset(flashBuffer256Bytes, 0, 256);
}


// Save the token and its parameters to flash
void saveTokenAndNumbersToFlash(uint8_t token, uint16_t *numbers, uint8_t numOfNumbers)
{
  // Show the token being written for debugging
  printfD("%s\n", tokenToText(buffer100Bytes, token, numbers));

  // Is this the end of the block?
  if (offsetIntoBlock > (256 - MAX_TOKEN_LENGTH)) {
    flashBuffer256Bytes[offsetIntoBlock] = TOKEN_NEXT_FLASH_BLOCK;
    writeTokenBufferToFlash(false);
  }

  // Save the token
  flashBuffer256Bytes[offsetIntoBlock++] = token;
  // Save the numbers
  for (uint8_t i=0; i< numOfNumbers; i++) {
    memcpy(flashBuffer256Bytes + offsetIntoBlock, numbers + i, 2);
    offsetIntoBlock += 2;
  }
}


// Save a string to flash.  Currently, only "Display" takes a string as
// a parameter.  The string is saved null-terminated.
void saveTokenAndStringToFlash(uint16_t token, char *str)
{
  // Show the token being written for debugging
  printfD("Display \"%s\"/n", str);

  // Is this the end of the block?
  if (offsetIntoBlock > (256 - MAX_TOKEN_LENGTH)) {
    flashBuffer256Bytes[offsetIntoBlock] = TOKEN_NEXT_FLASH_BLOCK;
    writeTokenBufferToFlash(false);
  }
  
  // Save the token
  flashBuffer256Bytes[offsetIntoBlock++] = token;
  // Save the string
  strcpy((char *) flashBuffer256Bytes + offsetIntoBlock, str);
  offsetIntoBlock+= strlen(str) + 1;
}


// Write the 256-byte profile block to flash
bool writeTokenBufferToFlash(bool endOfProfile)
{
  // Is this the end of the profile?
  if (endOfProfile)
    flashBuffer256Bytes[offsetIntoBlock] = TOKEN_END_OF_PROFILE;
  // Is this the last available block?  Profiles can take 16 blocks = 16 x 256-bytes = 4K
  if (numBlocksUsed == 15)
    flashBuffer256Bytes[offsetIntoBlock] = TOKEN_END_OF_PROFILE;
  
  if (numBlocksUsed < 16) {
    // Unprotect flash so writing can take place
    flash.allowWritingToPrefs(true);
    // Write the 256-byte block of tokens (only have 4K = 16x256 blocks available)
    flash.write(startFlashBlock + numBlocksUsed, 256, flashBuffer256Bytes);
    // Protect flash again
    flash.allowWritingToPrefs(false);
    printfD("Wrote profile flash block %d size (bytes) = %d", (startFlashBlock + numBlocksUsed), offsetIntoBlock);
  }

  // Initialize all the variables used to store the tokens
  numBlocksUsed++;
  offsetIntoBlock = 0;
  memset(flashBuffer256Bytes, 0, 256);

  if (numBlocksUsed > 16) {
    printfD("ERROR: Some tokens have been discarded!  Profile file is too long\n");
    return false;
  }
  return true;
}


// Get the next token from flash.  If str is smaller than 512 then it is the first
// block number and the flash reading must be initialized to that block.  It's
// a bit hacky, but means we can use reduced-scope static variables.
uint16_t getNextTokenFromFlash(char *str, uint16_t *num)
{
  uint16_t token;
  static uint16_t startBlock, offset, blocksRead = 0;
  // Need to use this buffer to ensure it lies on the 16-bit boundary to allow 16-bit access

  // Is this the start of the profile reading?
  if (!str && *num < 512) {
    startBlock = *num;
    offset = 0;
    blocksRead = 0;

    // Is the block number good?
    if ((startBlock & 0x0F) || startBlock < 64) {
      printfD("getNextTokenFromFlash: Profile block number out of range %d\n", startBlock);
      return TOKEN_END_OF_PROFILE;
    }

    // Read the first block from flash into memory
    printfD("getNextTokenFromFlash: Reading first block %d\n", startBlock);
    flash.startRead(startBlock, 256, flashBuffer256Bytes);
    flash.endRead();

    // Done for now.  The next time this is called a real token will be returned
    return NOT_A_TOKEN;
  }

  // Figure out what token parameters need to be returned
  token = flashBuffer256Bytes[offset];
  switch (token) {
    case TOKEN_DISPLAY:
      strcpy(str, (char *) flashBuffer256Bytes + offset + 1);
      offset += strlen((char *) (flashBuffer256Bytes + offset + 1)) + 2;
      break;

    case TOKEN_MAX_DUTY:
    case TOKEN_ELEMENT_DUTY_CYCLES:
    case TOKEN_BIAS:
      // This should be followed by 3 numbers, indicating bottom/top/boost
      memcpy(num, flashBuffer256Bytes + offset + 1, 6);
      offset += 7;
      break;
 
    case TOKEN_TEMPERATURE_TARGET:
    case TOKEN_OVEN_DOOR_PERCENT:
    case TOKEN_MAINTAIN_TEMP:
      // This should be followed by 2 numbers
      memcpy(num, flashBuffer256Bytes + offset + 1, 4);
      offset += 5;
      break;

    case TOKEN_DEVIATION:
    case TOKEN_MAX_TEMPERATURE:
    case TOKEN_INITIALIZE_TIMER:
    case TOKEN_OVEN_DOOR_OPEN:
    case TOKEN_OVEN_DOOR_CLOSE:
    case TOKEN_WAIT_FOR_SECONDS:
    case TOKEN_WAIT_UNTIL_ABOVE_C:
    case TOKEN_WAIT_UNTIL_BELOW_C:
      // These require 1 parameter
      memcpy(num, flashBuffer256Bytes + offset + 1, 2);
      offset += 3;
      break;
          
    case TOKEN_START_TIMER:
    case TOKEN_STOP_TIMER:
    case TOKEN_CONVECTION_FAN_ON:
    case TOKEN_CONVECTION_FAN_OFF:
    case TOKEN_COOLING_FAN_ON:
    case TOKEN_COOLING_FAN_OFF:
    case TOKEN_PLAY_DONE_TUNE:
    case TOKEN_PLAY_BEEP:
      // These don't take parameters
      offset ++;
      break;

    case TOKEN_END_OF_PROFILE:
      // Nothing to do, but don't advance over this token
      break;

    case TOKEN_NEXT_FLASH_BLOCK:
      // Sanity check
      if (blocksRead++ >= 16) {
        printfD("getNextTokenFromFlash: Too many blocks\n");
        token = TOKEN_END_OF_PROFILE;
        break;
      }
      // Read the next block from flash into memory
//      printfD("getNextTokenFromFlash: Reading extra block " + String(startBlock+ blocksRead));
      flash.startRead(startBlock + blocksRead, 256, flashBuffer256Bytes);
      flash.endRead();
      offset = 0;
      return getNextTokenFromFlash(str, num);

    default:
      // Should never get here
      printfD("getNextTokenFromFlash: Unknown token %d\n", token);
      token = TOKEN_END_OF_PROFILE;
      break;
  }
  return token;
}


// Dump profile for debugging
void dumpProfile(uint8_t profileNo)
{
  uint16_t block, token, numbers[4];
  
  // Sanity check
  if (profileNo >= MAX_PROFILES)
    return;

  // Get the start block of the profile.  The profile could be 16 blocks long (16 * 256 bytes = 4K)
  block = prefs.profile[profileNo].startBlock;

  // Set up the profile reading
  if (getNextTokenFromFlash(0, &block) == TOKEN_END_OF_PROFILE)
    return;

  printfD("---- Start of profile ----\n");

  while (1) {
    // Get the next token
    token = getNextTokenFromFlash(buffer100Bytes, numbers);

    // Display the token and parameters
    switch (token) {
      case TOKEN_DISPLAY:
        printfD("Display \"%s\"\n", buffer100Bytes);
        break;

      case TOKEN_MAX_DUTY:
      case TOKEN_ELEMENT_DUTY_CYCLES:
      case TOKEN_BIAS:
        // This should be followed by 3 numbers, indicating bottom/top/boost
        printfD("%s\n", tokenToText(buffer100Bytes, token, numbers));
        break;
 
      case TOKEN_TEMPERATURE_TARGET:
      case TOKEN_OVEN_DOOR_PERCENT:
      case TOKEN_MAINTAIN_TEMP:
        // These should be followed by 2 numbers
        printfD("%s\n", tokenToText(buffer100Bytes, token, numbers));
        break;

      case TOKEN_DEVIATION:
      case TOKEN_MAX_TEMPERATURE:
      case TOKEN_INITIALIZE_TIMER:
      case TOKEN_OVEN_DOOR_OPEN:
      case TOKEN_OVEN_DOOR_CLOSE:
      case TOKEN_WAIT_FOR_SECONDS:
      case TOKEN_WAIT_UNTIL_ABOVE_C:
      case TOKEN_WAIT_UNTIL_BELOW_C:
        // These require 1 parameter
        printfD("%s\n", tokenToText(buffer100Bytes, token, numbers));
        break;
          
      case TOKEN_START_TIMER:
      case TOKEN_STOP_TIMER:
      case TOKEN_CONVECTION_FAN_ON:
      case TOKEN_CONVECTION_FAN_OFF:
      case TOKEN_COOLING_FAN_ON:
      case TOKEN_COOLING_FAN_OFF:
      case TOKEN_PLAY_DONE_TUNE:
      case TOKEN_PLAY_BEEP:
        // These don't take parameters
        printfD("%s\n", tokenToText(buffer100Bytes, token, numbers));
        break;

      case TOKEN_END_OF_PROFILE:
        printfD("---- End of profile ----\n");
        return;

      default:
        // Should never get here
        printfD("dumpProfile: Unknown token\n");
        return;
    }
  }
}


// Return true if a profile of the same name exists
bool profileExists(char *profileName)
{
  for (uint8_t i = 0; i< prefs.numProfiles; i++) {
    if (strcmp(profileName, prefs.profile[i].name) == 0)
      return true;
  }
  return false;
}


// Delete profiles having the same name so you don't get duplicates
void deleteProfileByName(char *profileName)
{
  printfD("Looking to delete profile: %s\n", profileName);
  for (uint8_t i = 0; i< prefs.numProfiles; i++) {
    if (strcmp(profileName, prefs.profile[i].name) == 0) {
      printfD("Deleting profile %d\n", i);
      deleteProfile(i);
      return;
    }
  }  
}


// Find the first flash block available to store a profile.  Profiles are stored in
// sequential blocks to avoid flash wear
uint16_t getFreeProfileBlock()
{
  uint16_t flashBlock = prefs.lastUsedProfileBlock;
  bool blockIsUsed;
  
  // Make sure there should be a free block
  if (prefs.numProfiles >= MAX_PROFILES)
    return 0;

  // Prevent an infinite loop if something went really screwy
  for (uint8_t i=0; i< MAX_PROFILES; i++)
  {
    // Figure out the next flash block to use
    flashBlock += PROFILE_SIZE_IN_BLOCKS;
    if (flashBlock > LAST_PROFILE_BLOCK)
      flashBlock = FIRST_PROFILE_BLOCK;
    blockIsUsed = false;

    // See if any existing profile is using this block
    for (uint8_t j=0; j < prefs.numProfiles; j++) {
      // If this profile uses this flash block then skip this block
      if (prefs.profile[j].startBlock == flashBlock) {
        blockIsUsed = true;
        break;
      }
    }

    // If this block isn't being used then bingo!
    if (!blockIsUsed) {
      printfD("Next unused flash block = %d\n", flashBlock);
      return flashBlock;
    }
  }

  // We should never get here unless there is a terrible mess in flash
  printfD("REALLY BAD: NO FREE FLASH BLOCK!\n");
  return 0;
}


// Keep profiles in alphabetical order, with unused profiles at the end
// Yes, inefficient and slow.  But how often is this called?  Only when
// profiles are read from the SD card (and the SD card is really slow!)
void sortProfiles(void) 
{
  profiles temp;
  for (uint8_t i=0; i < MAX_PROFILES-1; i++) {
    for (uint8_t j=i+1; j < MAX_PROFILES; j++) {
      // Handle the simple case first
      if (prefs.profile[j].startBlock == 0)
        continue;
      // If the first profile is not valid then switch them
      if (prefs.profile[i].startBlock == 0) {
        printfD("Moving %s (%d) to (%d)\n", prefs.profile[j].name, j, i);
        // Copy one block to the other
        memcpy(&prefs.profile[i], &prefs.profile[j], sizeof(struct profiles));
        // Zero out the old profile
        prefs.profile[j].startBlock = 0;
        continue;
      }
      // Both profiles are valid
      if (strcmp(prefs.profile[i].name, prefs.profile[j].name) <= 0)
        continue;
      // Swap the profiles
      printfD("Swapping %s (%d and %d)\n", prefs.profile[i].name, i, j);
      memcpy(&temp, &prefs.profile[i], sizeof(struct profiles));
      memcpy(&prefs.profile[i], &prefs.profile[j], sizeof(struct profiles));
      memcpy(&prefs.profile[j], &temp, sizeof(struct profiles));
    }
  }
  printfD("Profiles have been sorted\n");
  prefs.numProfiles = getNumberOfProfiles();
}


// Delete the selected profile
// Erase the profile block so that it can be used again, delete
// the profile then sort the remaining ones.
void deleteProfile(uint8_t num)
{
  // Sanity check on parameter
  if (num >= MAX_PROFILES)
    return;
  // Firstly, delete the flash blocks used to store the profile
  if (prefs.profile[num].startBlock)
    flash.eraseProfileBlock(prefs.profile[num].startBlock);
  // Set the block to zero in the prefs to indicate "not used"
  prefs.profile[num].startBlock = 0;
  // Profiles need to be sorted to account for the deleted profile
  sortProfiles();
  // Recalculate the number of valid profiles
  // Yes, we could just decrement numProfiles but this is safer because we're dealing with flash
  prefs.numProfiles = getNumberOfProfiles();
  // Save the prefs
  savePrefs();
  printfD("Deleted profile %d  No. of profiles= %d\n", num, prefs.numProfiles);
}


// Get the number of profiles.
uint8_t getNumberOfProfiles()
{
  uint8_t count = 0;
  
  for (uint8_t i = 0; i< MAX_PROFILES; i++) {
    // If the profile has a non-zero start block then it is valid
    if (prefs.profile[i].startBlock != 0)
      count++;
  }
  return count;  
}

