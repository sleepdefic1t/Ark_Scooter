
#include "eeprom.hpp"

#include "rental.hpp"

/*******************************************************************************
 * Library for reading/writing to the ESP32 flash memory.
 * ESP32 Arduino libraries emulate EEPROM using a sector (4 kilobytes) of flash
 * memory. The total flash memory size is ??? The entire space is split between
 * bootloader, application, OTA data, NVS, SPIFFS, and EEPROM. EEPROM library on
 * the ESP32 allows using at most 1 sector (4kB) of flash.
 ******************************************************************************/
#include <EEPROM.h>

/*******************************************************************************
 * Copy data from nonvolatile memory into RAM.
 *
 * Note. ESP32 has FLASH memory(not EEPROM) however the standard high level
 * Arduino EEPROM arduino functions work.
 *
 * To load data from EEPROM, EEPROM.begin(size) must be called before reading or
 * writing. Size is the number of bytes you want to use and can be anywhere
 * between 4 and 4096 bytes.
 ******************************************************************************/
int loadEEPROM() {
  EEPROM.begin(512);
  int RXpage = 0;
  EEPROM.get(0, RXpage);

  char ok[2 + 1];
  EEPROM.get(0 + sizeof(RXpage), ok);
  EEPROM.end();

  if (String(ok) != String("OK")) {
    RXpage = 0;
  }

  Serial.printf("\nRecovered RXpage '%d' from FLASH: %s | ", RXpage, ok);

  return RXpage;
}

/*******************************************************************************
 * Store data in nonvolatile memory.
 *
 * Note. ESP32 has FLASH memory(not EEPROM) however the standard high level
 * Arduino EEPROM arduino functions work. EEPROM.write does not write to flash
 * immediately, instead you must call EEPROM.commit() whenever you wish to save
 * changes to flash. EEPROM.end() will also commit, and will release the RAM
 * copy of EEPROM contents.
 ******************************************************************************/
void saveEEPROM(int RXpage) {
  EEPROM.begin(512);
  EEPROM.put(0, RXpage);

  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(RXpage), ok);
  EEPROM.commit();

  EEPROM.end();

  Serial.println("Saved credentials to FLASH");
}

#ifdef ERASE_FLASH
/*******************************************************************************
 * Clear data in nonvolatile memory.
 *
 * Note:
 * ESP32 has FLASH memory(not EEPROM).
 * However, the standard high level Arduino EEPROM arduino functions work.
 *
 * EEPROM.write does not write to flash immediately.
 *
 * EEPROM.commit() must be called to save changes to flash.
 *
 * EEPROM.end() will also commit, and releases the RAM copy of EEPROM contents.
 ******************************************************************************/
void clearEEPROM() {
  EEPROM.begin(512);
  EEPROM.put(0, 0);
  EEPROM.put(1, 0);
  EEPROM.put(2, 0);
  EEPROM.put(3, 0);
  EEPROM.commit();
  EEPROM.end();
  Serial.println("cleared FLASH");
}
#endif
