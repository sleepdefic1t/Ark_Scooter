/*******************************************************************************
 * Proof of Concept Scooter Rental
 * https://hackmd.io/j3Ha1rlVQp-jTyyw9MU8FQ?both#Functional-Requirements
 *
 * Create an electric scooter rental solution utilizing a mobile app and an
 * associated Ark custom bridgechain. All communication between the App and the
 * IOT device will be on chain through the use of custom transactions.
 *
 * Ark_Scooter.ino
 * 2020 @phillipjacobsen
 *
 *
 * Program Features:
 *  This program is designed to run on ESP32 Adafruit Huzzah.
 *  This sketch uses the ARK Cpp-Client API to interact with a custom Ark V2.6
 *  bridgechain. Ark Cpp Client available from Ark Ecosystem <info@ark.io> Ark
 *
 * API documentation:  https://docs.ark.io/sdk/clients/usage.html
 *
 *
 * Electronic Hardware Peripherals:
 * Adafruit TFT FeatherWing 2.4" 320x240 Touchscreen:
 *  https://www.adafruit.com/product/3315
 * Adafruit GPS FeatherWing:
 *  https://www.adafruit.com/product/3133
 * Optional: GPS external antenna:
 *  https://www.adafruit.com/product/960
 * Optional(Required when using external antenna): SMA to uFL cable:
 *  https://www.adafruit.com/product/851 Adafruit
 * FeatherWing Doubler:
 *  https://www.adafruit.com/product/2890
 ******************************************************************************/

#include <Arduino.h>

#include "ark_scooters.hpp"
#include "secrets.hpp"

/*******************************************************************************
 * Mqtt Client
 ******************************************************************************/
 #include "EspMQTTClient.h"

static EspMQTTClient WiFiMQTTclient(
    WIFI_SSID, WIFI_PASS,
    MQTT_SERVER_IP,    // MQTT Broker server ip
    MQTT_USERNAME,     // Can be omitted if not needed
    MQTT_PASSWORD,     // Can be omitted if not needed
    MQTT_CLIENT_NAME,  // Client name that uniquely identifies your device
    MQTT_SERVER_PORT   // The MQTT port, default: 1883. this line can be omitted
);


/*******************************************************************************
 * Create a Blockchain API connection
 ******************************************************************************/
#include <arkClient.h>

Ark::Client::Connection<Ark::Client::Api> connection(ARK_PEER, ARK_PORT);

/*******************************************************************************
 * MAIN SETUP
 ******************************************************************************/
void setup() {
  /*******************************************************************************
   * If reprogramming a new wallet address into an already-used ESP32 module,
   * Flash storage--which contains the Wallets 'Nonce'--needs erased first.
   *
   * - 1. Define ERASE_FLASH in secrets.hpp
   * - 2. Download firmware
   * - 3. undefine ERASE_FLASH and reprogram
   ******************************************************************************/
#ifdef ERASE_FLASH
  clearEEPROM();
#endif

  Serial.begin(115200);  // Initialize Serial Connection for debug
  while (!Serial && millis() < 20) {
    Serial.print(".");
  };

  // initialize on board LED control pin as an output.
  pinMode(LED_PIN, OUTPUT);
  // Turn LED on
  digitalWrite(LED_PIN, HIGH);

  // read the MAC address.
  // https://cpp4arduino.com/2018/11/21/eight-tips-to-use-the-string-class-efficiently.html
  uint8_t baseMac[6];
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);

  char baseMacChr[18] = {0};
  Serial.printf("\nMAC Address: %02X%02X%02X%02X%02X%02X", baseMac[0],
                baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

  configureMqttClient(WiFiMQTTclient, MQTT_Base_Topic);

  // configure the 2.4" TFT display and the touchscreen controller
  initDisplay();

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS
  GPS.begin(9600);

  // turn on RMC (recommended minimum) and  GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  // For the parsing code to work nicely and have time to sort thru the data,
  // and print it out we don't suggest using anything higher than 1 Hz
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  // 1 Hz update rate

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  startDisplay(WIFI_SSID);

  // request firmware version from GPS module
  GPSSerial.println(PMTK_Q_RELEASE);

  setConfiguration(BRIDGECHAIN_NETHASH, BRIDGECHAIN_SLIP44, BRIDGECHAIN_WIF,
                   BRIDGECHAIN_VERSION, BRIDGECHAIN_EPOCH);
  setScooterVars(RENTAL_RATE_STR, RENTAL_RATE_UINT64, TIME_ZONE, DST,
                 &WiFiMQTTclient, connection);
  setStateMachineVars(ARK_ADDRESS, RENTAL_RATE_STR, RENTAL_RATE_UINT64);
  setTransactionVars(ARK_ADDRESS, PASSPHRASE, TYPE_0_FEE);
}

/*******************************************************************************
 * MAIN LOOP
 * ******************************************************************************/
void loop() {
  // Process state machine
  StateMachine(connection);

  // Handle the WiFi and MQTT connections
  WiFiMQTTclient.loop();

  // Parse GPS data if available
  // We need to call GPS.read() constantly in the main loop to watch for data
  // arriving on the serial port The hardware serial port has some buffer and
  // perhaps arduino also configures some sort of FIFO.  This may set the buffer
  // size???: Serial1.setRxBufferSize(1024); I need to learn more about the
  // hardware buffer available on the ESP32 serial port.
  char c = GPS.read();

  // ask if a new data chunk has been received by calling GPS.newNMEAreceived().
  // if this returns true,
  // then we can ask the library to parse it with GPS.parse(GPS.lastNMEA()).
  if (GPS.newNMEAreceived() && GPS.parse(GPS.lastNMEA())) {
    // this also sets the newNMEAreceived() flag to false
    // we can fail to parse, in which case we should just wait for another
    return;
  }

  // Update all the data displayed on the TFT Display Status Bar
  // update WiFi status bar
  UpdateWiFiConnectionStatus(WiFiMQTTclient.isWifiConnected(), WiFi_status);
  // update MQTT status bar
  UpdateMQTTConnectionStatus(MQTT_status, WiFiMQTTclient);
  UpdateGPSConnectionStatus(GPS_status);  // update GPS status bar
  UpdateGPSDataStatus();                  // update GPS SAT and GPS Speed
  UpdateDisplayTime();                    // update the clock every 1 second

  // update battery status every UpdateInterval_Battery (5 seconds)
  UpdateBatteryStatus(batteryPercent);

  // update battery status every UpdateInterval_RSSI (5 seconds)
  UpdateRSSIStatus(WiFiMQTTclient.isWifiConnected(), WiFi.RSSI());

  // Publish MQTT data every UpdateInterval_MQTT_Publish (15 seconds)
  send_MQTTpacket(batteryPercent, scooterRental.rentalStatus, GPS,
                  bridgechainWallet.walletBalance, PASSPHRASE, MQTT_Base_Topic,
                  WiFiMQTTclient);
}
