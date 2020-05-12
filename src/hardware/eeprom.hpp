
#ifndef ARK_SCOOTERS_HARDWARE_EEPROM_HPP
#define ARK_SCOOTERS_HARDWARE_EEPROM_HPP

/*******************************************************************************
 * Copy data from nonvolatile memory into RAM.
 ******************************************************************************/
int loadEEPROM();

/*******************************************************************************
 * Store data in nonvolatile memory.
 ******************************************************************************/
void saveEEPROM(int RXpage);

#ifdef ERASE_FLASH
/*******************************************************************************
 * Clear data in nonvolatile memory.
 ******************************************************************************/
void clearEEPROM();
#endif

#endif  // ARK_SCOOTERS_HARDWARE_EEPROM_HPP