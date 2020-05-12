
#ifndef ARK_SCOOTERS_HARDWARE_GPS_HPP
#define ARK_SCOOTERS_HARDWARE_GPS_HPP

/*******************************************************************************
 * Adafruit GPS Library
 ******************************************************************************/
#include <Adafruit_GPS.h>

#define GPSSerial Serial1

// Connect to the GPS module via the hardware serial port
extern Adafruit_GPS GPS;

/*******************************************************************************
 * if GPS fix then update display with GPS Speed and GPS Sat
 ******************************************************************************/
void UpdateGPSDataStatus();

/*******************************************************************************
 * This routine will update the GPS Network Connection Icon on the TFT display
 * Display only updates on connection status change
 * When losing fix: Clear # satellites display. Clear Speed display
 * When obtaining fix: Show # satellites. Show Speed
 * Green Circle = GPS Fix achieved
 * Red Circle = GPS Fix not available
 ******************************************************************************/
void UpdateGPSConnectionStatus(bool gpsStatus);

#endif  // ARK_SCOOTERS_HARDWARE_GPS_HPP
