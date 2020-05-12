
#ifndef ARK_SCOOTERS_GPS_HPP
#define ARK_SCOOTERS_GPS_HPP

#include "gps.hpp"

#include "display.hpp"
#include "fonts/fonts.hpp"
#include "timer.hpp"

/*******************************************************************************
 * Adafruit GPS Library
 ******************************************************************************/
#include <Adafruit_GPS.h>

#define GPSSerial Serial1
// Connect to the GPS module via the hardware serial port
Adafruit_GPS GPS(&GPSSerial);

// void initGps() {

// }

/*******************************************************************************
 * if GPS fix then update display with GPS Speed and GPS Sat
 ******************************************************************************/
void UpdateGPSDataStatus() {
  if (GPS.fix) {
    if (millis() - previousUpdateTime_GPS > UpdateInterval_GPS) {
      previousUpdateTime_GPS += UpdateInterval_GPS;

      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      // clear the last GPS Sat reading
      tft.fillRect(190, 319 - 17, 40, 18, ILI9341_BLACK);
      tft.setCursor(190, 319);
      tft.print(GPS.satellites);

      // clear the last speed reading
      tft.fillRect(0, 283 - 17, 35, 18, ILI9341_BLACK);
      tft.setCursor(0, 283);
      float speedkmh = GPS.speed * 1.852;
      tft.print(speedkmh, 1);
    }
  }
}

/*******************************************************************************
 * This routine will update the GPS Network Connection Icon on the TFT display
 * Display only updates on connection status change
 * When losing fix: Clear # satellites display. Clear Speed display
 * When obtaining fix: Show # satellites. Show Speed
 * Green Circle = GPS Fix achieved
 * Red Circle = GPS Fix not available
 ******************************************************************************/
void UpdateGPSConnectionStatus(bool gpsStatus) {
  if (GPS.fix) {
    if (!gpsStatus) {
      tft.fillCircle(50, 319 - 6, 6,
                     GREEN);  // x,y,radius,color     //GPS Status

      tft.fillRect(190, 319 - 17, 40, 18,
                   BLACK);  // clear the last # of satellite reading
      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      tft.setCursor(190, 319);
      tft.print(GPS.satellites);

      tft.fillRect(0, 283 - 17, 35, 18, BLACK);  // clear the last speed reading
      tft.setCursor(0, 283);
      float speedkmh = GPS.speed * 1.852;
      tft.print(speedkmh, 1);

      gpsStatus = true;
    }
  } else {
    if (gpsStatus) {
      tft.fillCircle(50, 319 - 6, 6, RED);  // x,y,radius,color     //GPS Status
      tft.fillRect(190, 319 - 17, 40, 18,
                   BLACK);  // clear the last # of satellite reading
      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      tft.setCursor(190, 319);
      tft.print('0');

      tft.fillRect(0, 283 - 17, 35, 18, BLACK);  // clear the last speed reading
      tft.setCursor(0, 283);
      tft.print('0');

      gpsStatus = false;
    }
  }
}

#endif  // ARK_SCOOTERS_GPS_HPP
