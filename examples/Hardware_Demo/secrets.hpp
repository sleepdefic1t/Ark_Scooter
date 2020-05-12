
#ifndef ARK_SCOOTERS_SECRETS_H
#define ARK_SCOOTERS_SECRETS_H

#include <cstdint>

#define MQTT_SERVER_IP      "40.85.223.207"
#define MQTT_USERNAME       "esp32"
#define MQTT_PASSWORD       "compost2"
#define MQTT_CLIENT_NAME    "TestClient"
#define MQTT_SERVER_PORT    1883

#define WIFI_SSID   "*****"
#define WIFI_PASS   "*****"

// Todo: try using this library to handle daylight savings:
// https://github.com/JChristensen/Timezone

// set timezone:  MST (works in summer)
// #define TIME_ZONE    uint8_t(-6);

// set timezone:  MST (works in winter)
#define TIME_ZONE   uint8_t(-7)

// To enable Daylight saving time set it to 3600.
// Otherwise, set it to 0. This does not seem to work.
#define DST     uint8_t(0)

// RADIANS Testnet Peer
#define ARK_PEER    "37.34.60.90"
#define ARK_PORT    4040

// Configure Wallet info for Scooter
// RADIANS testnet address. nickname: pjtest
#define ARK_ADDRESS     "TRXA2NUACckkYwWnS9JRkATQA453ukAcD1"
#define PASSPHRASE      "afford thumb forward wall salad diet title patch holiday metal cement wisdom"

#define MQTT_Base_Topic "scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/data"

// Configure Bridgechain Parameters
#define BRIDGECHAIN_NETHASH     "f39a61f04d6136a690a0b675ef6eedbd053665bd343b4e4f03311f12065fb875"
#define BRIDGECHAIN_SLIP44      uint8_t(1)
#define BRIDGECHAIN_WIF         uint8_t(0xce)
#define BRIDGECHAIN_VERSION     uint8_t(0x41)
#define BRIDGECHAIN_EPOCH       "2019-10-25T09:05:40.856Z"

#define TYPE_0_FEE      1000000ULL
#define TYPE_0_TYPE     uint8_t(0)

// Configure the Rental rate of the scooter.  Units are RAD per Second
#define RENTAL_RATE_STR     "61667"     // rate per second
#define RENTAL_RATE_UINT64  61667ULL    //.037 RAD

/*******************************************************************************
 * If reprogramming a new wallet address into an already-used ESP32 module,
 * Flash storage--which contains the Wallets 'Nonce'--needs erased first.
 *
 * - 1. Define ERASE_FLASH in secrets.hpp
 * - 2. Download firmware
 * - 3. undefine ERASE_FLASH and reprogram
 ******************************************************************************/
//#define ERASE_FLASH

/*******************************************************************************
 * Wireless Firmware Updating.
 *
 * - 1. to generate .bin firmware image,
 *      go to "Export Compiled Binary" in the Arduino IDE's "Sketch" menu.
 * - 2. In Arduino IDE go to tools->Partition Scheme,
 *      set to: Minimal SPIFFS(Large APPS with OTA).
 * - 3. Go to http://IPaddress
 * - 4. IPaddress of the ESP32 module is displayed on terminal after powerup.
 * - 5. Enter MQTT_USERNAME and MQTT_PASSWORD upload .bin file
 ******************************************************************************/
#define ENABLE_WIRELESS_UPDATE

#endif  // ARK_SCOOTERS_SECRETS_H
