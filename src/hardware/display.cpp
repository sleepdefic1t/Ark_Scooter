
#ifndef ARK_SCOOTERS_DISPLAY_DISPLAY_HPP
#define ARK_SCOOTERS_DISPLAY_DISPLAY_HPP

#include "display.hpp"

#include <Arduino.h>
#include <SPI.h>

/*******************************************************************************
 * Libraries for ILI9341 2.4" 240x320 TFT FeatherWing display + touchscreen
 * http://www.adafruit.com/products/3315
 *
 * Adafruit GFX libraries
 * graphics primitives documentation:
 * https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
 * top left corner is (0,0)
 *
 * great tool for generating custom fonts.
 * http://oleddisplay.squix.ch/#/home
 ******************************************************************************/
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>   //hardware specific library for display
#include <Adafruit_STMPE610.h>  //hardware specific library for the touch sensor

#include "fonts/fonts.hpp"
#include "timer.hpp"

/*******************************************************************************
 * Misc I/O Definitions
 ******************************************************************************/
// const int LED_PIN = 13;  // LED integrated on Adafruit HUZZAH32 module
const int BAT_PIN = 35;  // ADC connected to Battery input pin (A13 = 35;)

// pin connections
#define STMPE_CS 32
#define TFT_CS 15
#define TFT_DC 33
#define SD_CS 14

#define Lcd_X 240  // configure your screen dimensions.
#define Lcd_Y 320  // configure your screen dimensions

// my calibrated touchscreen data.
// Note: Resistive touchscreen is currently not being used.
#define TS_MINX 3800  // adjust left side of screen.    // default: 3800
#define TS_MAXX 250   // adjust right side of screen.   // default: 100
#define TS_MINY 205   // default: 100
#define TS_MAXY 3750  // default: 3750
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// create TFT display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// create Touchscreen object
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// RGB565 Color Definitions
// This is a good tool for color conversions into RGB565 format
// http://www.barth-dev.de/online/rgb565-color-picker/
#define BLACK ILI9341_BLACK
#define WHITE ILI9341_WHITE
#define RED ILI9341_RED
#define GREEN ILI9341_GREEN
#define ArkRed 0xF1A7            // rgb(241, 55, 58)
#define ArkLightRed 0xFCD3       // rgb(248, 155, 156)
#define OffWhite 0xCE59          // rgb(202, 202, 202)
#define SpeedGreen 0xAFF5        // rgb(170, 255, 170)
#define SpeedGreenDarker 0x0760  // rgb(0, 236, 0)
#define QRCODE_DARK_PIXEL_COLOR 0xF1A7

int batteryPercent = 0;  // use to store battery level in percentage
float previousSpeed = 0;

/*******************************************************************************
 * Prepare the Display
 *******************************************************************************/
void initDisplay() {
  // configure the 2.4" TFT display and the touchscreen controller
  setupDisplayTouchscreen();

  // Bootup Screen
  // Display Ark bitmap on middle portion of screen
  DisplayArkBitmap();
}

/*******************************************************************************
 * Start the Display
 *******************************************************************************/
void startDisplay(const char *ssid) {
  // show bootup screen for 500ms
  delay(500);

  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(50, 280);
  tft.println("Connecting to WiFi");
  tft.setCursor(70, 300);
  tft.println(ssid);

  // show bootup screen for additional 1200ms
  delay(1200);

  // configure the 2.4" TFT display and the touchscreen controller
  setupDisplayTouchscreen();

  // Bootup Screen
  // Display Ark bitmap on middle portion of screen
  DisplayArkBitmap();
}

/*******************************************************************************
 * Draw the status bar at the bottom of the screen
 *******************************************************************************/
void InitStatusBar() {
  // clear the status bar area + powered by ark.io text above it
  tft.fillRect(0, 265 - 20, 240, 55 + 20, BLACK);
  tft.setTextColor(ArkRed);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(45, 260);
  tft.print("Powered by Ark.io");
  tft.setTextColor(WHITE);

  tft.setCursor(60 - 21, 283);
  tft.print("kmh");

  tft.setCursor(0, 301);
  tft.print("WiFi");

  tft.setCursor(70, 301);
  tft.print("MQTT");

  tft.setCursor(0, 319);
  tft.print("GPS");

  tft.setCursor(70, 319);
  tft.print("ARK");

  tft.setCursor(150, 283);
  tft.print("RSSI");

  tft.setCursor(150, 301);
  tft.print("BAT");

  tft.setCursor(150, 319);
  tft.print("SAT");

  tft.fillCircle(50, 301 - 6, 6, RED);   // x,y,radius,color  // WiFi Status
  tft.fillCircle(130, 301 - 6, 6, RED);  // x,y,radius,color  // MQTT Status
  tft.fillCircle(50, 319 - 6, 6, RED);   // x,y,radius,color  // GPS Status
  tft.fillCircle(130, 319 - 6, 6, RED);  // x,y,radius,color  // ARK Status
}

/*******************************************************************************
 * Configures the TFT display and resistive touchscreen
 * There is also LITE pin which is not connected to any pads but you can use to
 * control the backlight. Pull low to turn off the backlight. You can connect it
 * to a PWM output pin. There is also an IRQ pin which is not connected to any
 * pads but you can use to detect when touch events have occurred. There is also
 * an Card Detect (CD) pin which is not connected to any pads but you can use to
 * detect when a microSD card has been inserted have occurred. It will be
 *shorted to ground when a card is not inserted.
 *******************************************************************************/
void setupDisplayTouchscreen() {
  // To replace previously-drawn text when using a custom font, use the
  // getTextBounds() function to determine the smallest rectangle encompassing a
  // string, erase the area using fillRect(), then draw new text.
  // https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts

  // setup 240x320 TFT display with custom font and clear screen
  tft.begin();

  // clear thescreen
  tft.fillScreen(BLACK);

  // 9pt = 12pixel height ??
  // https://reeddesign.co.uk/test/points-pixels.html
  tft.setFont(&FreeSans9pt7b);

  tft.setTextColor(WHITE);
}

/*******************************************************************************
 * Display Ark Splash Screen
 ******************************************************************************/
void DisplayArkBitmap() {
  clearMainScreen();
  // Display Ark bitmap on middle portion of screen
  tft.drawBitmap(56, 100, ArkBitmap, 128, 128, ArkRed);
}

/*******************************************************************************
 * Clears the screen except for the status bar
 ******************************************************************************/
void clearMainScreen() { tft.fillRect(0, 0, 240, 265 - 20, BLACK); }

/*******************************************************************************
 * read the battery voltage and update status bar
 * the ESP32 ADC should really be calibrated so these readings are good for
 * relative measurements.
 ******************************************************************************/
void UpdateBatteryStatus(int battPercent) {
  if (millis() - previousUpdateTime_Battery > UpdateInterval_Battery) {
    previousUpdateTime_Battery += UpdateInterval_Battery;

    int battery = analogRead(BAT_PIN);
    battPercent = map(battery, 1945, 2348, 0, 100);
    battPercent = constrain(battPercent, 0, 100);

    // we needed to add fudge factor to calibrate readings.
    // There must not be a 50% voltage divider on the input.
    float batteryFloat = battery / 559.5;

    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(WHITE);

    // clear the last voltage reading
    tft.fillRect(190, 301 - 17, 40, 19, BLACK);
    tft.setCursor(190, 301);

    tft.print(batteryFloat);
    tft.print("V");
  }
}

/*******************************************************************************
 * update the clock on the status bar
 *
 * https://github.com/esp8266/Arduino/issues/4749
 * http://www.cplusplus.com/reference/ctime/strftime/
 ******************************************************************************/
void UpdateDisplayTime() {
  time_t now = time(nullptr);  // get current time

  if (now > 1500000000) {
    // this is a check to see if NTP time has been synced.
    // (this is a time equal to approximately the current time)
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    if ((timeinfo->tm_min) != prevDisplayMinute) {
      // update the display only if time has changed (updates every minute)
      prevDisplayMinute = timeinfo->tm_min;

      Serial.print("\ntime is: ");
      Serial.println(now);

      char formattedTime[30];
      strftime(formattedTime, 30, "%R", timeinfo);
	  Serial.print("24hr time: ");
      Serial.println(formattedTime);

      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);

      // clear the previous time
      tft.fillRect(70 + 13, 283 - 17, 65, 18, BLACK);
      tft.setCursor(70 + 13, 283);

      tft.print(formattedTime);  // dislay the current time
    }
  }
}

/*******************************************************************************
 * Updates the ride countdown timer Displayed on the screen.
 * It only refreshes the screen once per second
 ******************************************************************************/
void updateCountdownTimer() {
  // calculate remaining ride time in ms
  uint32_t remainingRentalTime_s =
      rideTime_length_ms - (millis() - rideTime_start_ms);

  if (remainingRentalTime_s > rideTime_length_ms) {
    // checks for wrap of unsigned type
    remainingRentalTime_s = 0;
  } else {
    remainingRentalTime_s = remainingRentalTime_s / 1000;  //# of seconds
  }

  if (remainingRentalTime_s != remainingRentalTime_previous_s) {
    // update display every second

    // create the string this is currently displayed on the screen.  We are
    // going to use this to calculate how much of the screen to erase
    char previousTimer_char[10];

    // create string from unsigned int
    snprintf(&previousTimer_char[0], 10, "%u", remainingRentalTime_previous_s);

    // create the string that we want to write to the display.
    char currentTimer_char[10];
    // create string from unsigned int
    snprintf(&currentTimer_char[0], 10, "%u", remainingRentalTime_s);

    // update previous timer update countdown timer display
    remainingRentalTime_previous_s = remainingRentalTime_s;
    int16_t x1, y1;
    uint16_t w, h;
    tft.setFont(&Lato_Semibold_48);
    tft.setTextColor(OffWhite);

    // get bounds of the previous speed text
    tft.getTextBounds(previousTimer_char, 70, 230, &x1, &y1, &w, &h);

    // erase the last speed reading
    tft.fillRect(x1, y1, w, h, BLACK);

    // display the updated countdowntimer on the display
    tft.setCursor(70, 230);
    tft.print(remainingRentalTime_s);
  }
}

/*******************************************************************************
 * Updates the speedometer displayed on the screen.
 * It only refreshes the screen if the speed changes
 ******************************************************************************/
void updateSpeedometer(float speedkmh) {
  char previousSpeed_char[6];
  // create string of previous speed with 1 decimal point.
  snprintf(&previousSpeed_char[0], 6, "%.1f", previousSpeed);

  char currentSpeed_char[6];

  // create string with 1 decimal point.
  snprintf(&currentSpeed_char[0], 6, "%.1f", speedkmh);

  if (strcmp(currentSpeed_char, previousSpeed_char) == 0) {
    return;
  }

  int16_t x1, y1;
  uint16_t w, h;
  tft.setFont(&Lato_Black_96);

  // get bounds of the previous speed text
  tft.getTextBounds(previousSpeed_char, 30, 105, &x1, &y1, &w, &h);

  // clear the last speed reading
  tft.fillRect(x1, y1, w, h, BLACK);

  // display updated speed
  previousSpeed = speedkmh;  // update previous speed
  tft.setFont(&Lato_Black_96);
  tft.setTextColor(SpeedGreenDarker);
  tft.setCursor(30, 105);
  tft.print(speedkmh, 1);
}

/*******************************************************************************
 * read the WiFi RSSI and update status bar
 * I don't know if the value can actually be measured as actual dBm or just some
 * other relative measurement
 ******************************************************************************/
void UpdateRSSIStatus(bool isConnected, long rssi) {
  if (millis() - previousUpdateTime_RSSI > UpdateInterval_RSSI) {
    previousUpdateTime_RSSI += UpdateInterval_RSSI;
    if (isConnected) {
      // clear the last voltage reading
      tft.fillRect(195, 283 - 18, 40, 20, BLACK);
      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      tft.setCursor(195, 283);
      tft.print(rssi);
    }
  }
}

/*******************************************************************************
 * Update status bar with ARK node connection status.
 ******************************************************************************/
void UpdateArkNodeConnectionStatus(bool isNodeOkay, bool status) {
  if (isNodeOkay) {
    if (!status) {
      // x,y,radius,color  //ARK Status
      tft.fillCircle(130, 319 - 6, 6, GREEN);
      status = true;
    }
  } else {
    if (status) {
      // x,y,radius,color  //ARK Status
      tft.fillCircle(130, 319 - 6, 6, RED);
      status = false;
    }
  }
}

/*******************************************************************************
 * Update status bar with WifI connection status
 * Display updates only when connection status change
 ******************************************************************************/
void UpdateWiFiConnectionStatus(bool isConnected, bool status) {
  if (isConnected) {
    if (!status) {
      // x,y,radius,color  //WiFi Status
      tft.fillCircle(50, 301 - 6, 6, GREEN);
      status = true;
    }
  } else {
    if (status) {
      // x,y,radius,color  //WiFi Status
      tft.fillCircle(50, 301 - 6, 6, RED);
      status = false;
    }
  }
}

#endif  // ARK_SCOOTERS_DISPLAY_DISPLAY_HPP
