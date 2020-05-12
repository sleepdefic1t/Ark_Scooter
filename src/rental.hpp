/*******************************************************************************
  This structure is used to store all the details of a Rental session
 ******************************************************************************/

#ifndef ARK_SCOOTERS_RENTAL_HPP
#define ARK_SCOOTERS_RENTAL_HPP

#include <cstdint>

typedef struct Rental {
  const char* rentalStatus;  // Options: Available, Broken, Rented, Charging
  char senderAddress[34 + 1];
  char payment[64 + 1];
  uint64_t payment_Uint64;
  char rentalRate[64 + 1];
  uint64_t rentalRate_Uint64;
  float QRLatitude;
  float QRLongitude;
  uint32_t startTime;
  float startLatitude;
  float startLongitude;
  uint32_t endTime;
  float endLatitude;
  float endLongitude;
  char vendorField[256 + 1];
  char sessionID_RentalStart[64 + 1];
  char sessionID_QRcode[64 + 1];
  uint8_t sessionID_QRcode_byte[32];
} ScooterRental;

extern ScooterRental scooterRental;

/********************************************************************************
  This structure is used to store details of the bridgechain wallet
********************************************************************************/
typedef struct Wallet {
  char walletBalance[64 + 1];            // current balance
  uint64_t walletBalance_Uint64;  // current balance
  char walletNonce[64 + 1];              // current nonce
  uint64_t walletNonce_Uint64;    // current nonce
  int lastRXpage;  // page number of the last received transaction in wallet
} BridgechainWallet;

extern BridgechainWallet bridgechainWallet;

#endif  // ARK_SCOOTERS_RENTAL_HPP
