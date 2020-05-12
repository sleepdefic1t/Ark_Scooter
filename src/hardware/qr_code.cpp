
#include "qr_code.hpp"

#include <cstdint>

#include "display.hpp"
#include "fonts/fonts.hpp"

// set the version (range 1->40)
const int QRcode_Version = 10;

// set the Error Correction level (range 0-3)
// or symbolic (ECC_LOW, ECC_MEDIUM, ECC_QUARTILE and ECC_HIGH)
const int QRcode_ECC = 1;

// Create the QR code object
QRCode qrcode;

char QRcodeHash[64 + 1] = { '\0' };
uint8_t shaResult[32] = { 0 };


/*******************************************************************************
 * This routine will display a large QRcode on a 240x320 TFT display
 ******************************************************************************/
void displayQRcode(char *const QRcodeText) {
  clearMainScreen();
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(ArkRed);
  tft.setCursor(12, 30);
  tft.print("Scan to Ride");
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(WHITE);

  // Allocate memory to store the QR code.
  // memory size depends on version number
  uint8_t qrcodeData[qrcode_getBufferSize(QRcode_Version)];

  qrcode_initText(&qrcode, qrcodeData, QRcode_Version, QRcode_ECC, QRcodeText);

  // white background with a few pixels of guard around the code
  tft.fillRoundRect(27, 77 - 30, 186, 186, 4, WHITE);

  // position the code.
  uint8_t x0 = 35;
  uint8_t y0 = 85 - 30;  //

  // display QRcode
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y) != 0) {
        // uncomment to double the QRcode. Comment to display normal code size
        tft.drawPixel(x0 + 3 * x, y0 + 3 * y, QRCODE_DARK_PIXEL_COLOR);
        tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y, QRCODE_DARK_PIXEL_COLOR);
        tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y, QRCODE_DARK_PIXEL_COLOR);

        tft.drawPixel(x0 + 3 * x, y0 + 3 * y + 1, QRCODE_DARK_PIXEL_COLOR);
        tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y + 1, QRCODE_DARK_PIXEL_COLOR);
        tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y + 1, QRCODE_DARK_PIXEL_COLOR);

        tft.drawPixel(x0 + 3 * x, y0 + 3 * y + 2, QRCODE_DARK_PIXEL_COLOR);
        tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y + 2, QRCODE_DARK_PIXEL_COLOR);
        tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y + 2, QRCODE_DARK_PIXEL_COLOR);
      }
    }
  }
}
