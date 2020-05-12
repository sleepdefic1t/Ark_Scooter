
#include "ark_scooters.hpp"

// Private Data
#include "ark_transactions.hpp"
#include "configuration.hpp"
#include "hardware/display.hpp"
#include "hardware/eeprom.hpp"
#include "hardware/gps.hpp"
#include "hardware/mqtt.hpp"
#include "rental.hpp"
#include "state_machine.hpp"
#include "timer.hpp"

/*******************************************************************************
 * Misc I/O Definitions
 ******************************************************************************/
const int LED_PIN = 13;  // LED integrated on Adafruit HUZZAH32 module

/*******************************************************************************
 * Various Global Variables
 ******************************************************************************/
// used to detect first run after power up
bool initialConnectionEstablished_Flag = false;

// = true when communication to Radians Bridgechain Node is working
bool ARK_status = false;
bool GPS_status = false;   // = true when GPS has signal lock
bool MQTT_status = false;  // = true when connected to MQTT broker
bool WiFi_status = false;  // = true when connected to WiFi access point

// scooter vars
char _scooterRateStr[20] = {0};
uint64_t _scooterRate64 = 0ULL;
uint8_t _timeZone = 0U;
uint8_t _dst = 0U;
EspMQTTClient *_client;

/*******************************************************************************
 * Set scooter variables.
 ******************************************************************************/
void setScooterVars(const char *rateStr, uint64_t rate64, uint8_t timeZone,
                    uint8_t dst, EspMQTTClient *client) {
  strcpy(_scooterRateStr, rateStr);
  _scooterRate64 = rate64;
  _timeZone = timeZone;
  _dst = dst;
  _client = client;
}

/*******************************************************************************
 * This function is called once WiFi and MQTT connections are complete.
 *
 * This is a callback function from the Mqtt Library.
 *******************************************************************************/
void onConnectionEstablished() {
  if (!initialConnectionEstablished_Flag) {
    // execute this the first time we have established a WiFi and MQTT
    // connection after powerup
    initialConnectionEstablished_Flag = true;

    // sync local time to NTP server
    // https://github.com/esp8266/Arduino/issues/4749  check this to see how to
    // TODO: Improve
    configTime(_timeZone * 3600, _dst, "pool.ntp.org", "time.nist.gov");

    Serial.printf("\nIP address: %s", WiFi.localIP());

    // update WiFi and MQTT connection status bar
    UpdateWiFiConnectionStatus(_client->isWifiConnected(), WiFi_status);
    UpdateMQTTConnectionStatus(MQTT_status, *_client);

    // --------------------------------------------
    // query Ark Node to see if it is synced
    // we need some error handling here!!!!!!  What do we do if there is no ark
    // node connected? if (checkArkNodeStatus()) {

    // Retrieve Wallet Nonce and Balance
    getWallet();

    // Copy data stored in Flash into RAM
    bridgechainWallet.lastRXpage =
        loadEEPROM();  // load page number from eeprom
    if (bridgechainWallet.lastRXpage < 1) {
      bridgechainWallet.lastRXpage = 0;
    }

    // Parse the wallet looking for the last received transaction
    // lastRXpage is equal to the page number of the last received transaction
    // in the wallet.
    bridgechainWallet.lastRXpage =
        getMostRecentReceivedTransaction(bridgechainWallet.lastRXpage + 1);
    saveEEPROM(bridgechainWallet.lastRXpage);

    // query Ark Node to see if it is synced and update status bar
    UpdateArkNodeConnectionStatus(checkArkNodeStatus(), ARK_status);

    scooterRental.rentalRate_Uint64 = _scooterRate64;
    strcpy(scooterRental.rentalRate, _scooterRateStr);

    // wait for time to sync from NTP servers
    while (time(nullptr) <= 100000) {
      delay(50);
    }

    // Update clock on TFT display
    UpdateDisplayTime();
  }

  // update WiFi and MQTT connection status bar
  UpdateWiFiConnectionStatus(_client->isWifiConnected(), WiFi_status);
  UpdateMQTTConnectionStatus(MQTT_status, *_client);
}
