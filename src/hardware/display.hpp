
#ifndef ARK_SCOOTERS_HARDWARE_DISPLAY_HPP
#define ARK_SCOOTERS_HARDWARE_DISPLAY_HPP

#include <SPI.h>

/*******************************************************************************
 * Libraries for ILI9341 2.4" 240x320 TFT FeatherWing display + touchscreen
 * http://www.adafruit.com/products/3315
 ******************************************************************************/
#include <Adafruit_ILI9341.h>   //hardware specific library for display
#include <Adafruit_STMPE610.h>  //hardware specific library for the touch sensor

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
extern Adafruit_ILI9341 tft;

// create Touchscreen object
extern Adafruit_STMPE610 ts;

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

extern int batteryPercent;
extern float previousSpeed;

/*******************************************************************************
 * Prepare the Display
 *******************************************************************************/
void initDisplay();

/*******************************************************************************
 * Start the Display
 *******************************************************************************/
void startDisplay(const char *ssid);

/*******************************************************************************
 * Draw the status bar at the bottom of the screen
 *******************************************************************************/
void InitStatusBar();

/*******************************************************************************
 * Configures the TFT display and resistive touchscreen

 *******************************************************************************/
void setupDisplayTouchscreen();

/*******************************************************************************
 * Display Ark Splash Screen
 ******************************************************************************/
void DisplayArkBitmap();

/*******************************************************************************
 * Clears the screen except for the status bar
 ******************************************************************************/
void clearMainScreen();

/*******************************************************************************
 * Read the battery voltage and update status bar
 ******************************************************************************/
void UpdateBatteryStatus(int battPercent);

/*******************************************************************************
 * update the clock on the status bar
 ******************************************************************************/
void UpdateDisplayTime();

/*******************************************************************************
 * Updates the ride countdown timer Displayed on the screen.
 * It only refreshes the screen once per second
 ******************************************************************************/
void updateCountdownTimer();

/*******************************************************************************
 * Updates the speedometer displayed on the screen.
 * It only refreshes the screen if the speed changes
 ******************************************************************************/
void updateSpeedometer(float speedkmh);

/*******************************************************************************
 * Read the WiFi RSSI and update status bar
 ******************************************************************************/
void UpdateRSSIStatus(bool isConnected, long rssi);

/*******************************************************************************
 * Update status bar with ARK node connection status.
 ******************************************************************************/
void UpdateArkNodeConnectionStatus(bool isNodeOkay, bool status);

/*******************************************************************************
 * Update status bar with WifI connection status
 * Display updates only when connection status change
 ******************************************************************************/
void UpdateWiFiConnectionStatus(bool isConnected, bool status);

#endif  // ARK_SCOOTERS_HARDWARE_DISPLAY_HPP
