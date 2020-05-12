
#ifndef ARK_SCOOTERS_HARDWARE_QR_CODE_HPP
#define ARK_SCOOTERS_HARDWARE_QR_CODE_HPP

/*******************************************************************************
 * QRCode by Richard Moore version 0.0.1
 * https://github.com/ricmoo/QRCode
 *
 * The QR code data encoding algorithm defines a number of 'versions' that
 * increase in size and store increasing amounts of data. The version (size) of
 * a generated QR code depends on the amount of data to be encoded. Increasing
 * the error correction level will decrease the storage capacity due to
 * redundancy pixels being added.
 *
 * If you have a ? in your QR text then I think the QR code operates in "Byte"
 * mode. QRcode Version = 10 with ECC=2 gives 211 Alphanumeric characters or 151
 * bytes(any characters).  ECC=1 = 213 characters
 ******************************************************************************/
#include "qrcode.h"

// set the version (range 1->40)
extern const int QRcode_Version;

// set the Error Correction level (range 0-3)
// or symbolic (ECC_LOW, ECC_MEDIUM, ECC_QUARTILE and ECC_HIGH)
extern const int QRcode_ECC;

// Create the QR code object
extern QRCode qrcode;

extern char QRcodeHash[64 + 1];
extern uint8_t shaResult[32];

/*******************************************************************************
 * This routine will display a large QRcode on a 240x320 TFT display
 ******************************************************************************/
void displayQRcode(char *const QRcodeText);

#endif  // ARK_SCOOTERS_HARDWARE_QR_CODE_HPP
