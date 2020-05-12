/*******************************************************************************
 * This contains Finite State Machine logic for controlling a rental session
 ******************************************************************************/

/*******************************************************************************
 * Improvements that could be done:
 *
 * Putting the transitional code in the state switch case makes your switch
 * statement hard to alter (have to change transitional code in multiple spots).
 *  https://www.reddit.com/r/programming/comments/1vrhdq/tutorial_implementing_state_machines/
 *  https://pastebin.com/22s5khze
 *
 *  Nice little tutorial on state machines:
 * https://www.embeddedrelated.com/showarticle/723.php
 ******************************************************************************/

#include "state_machine.hpp"

#include <cstdint>

#include "ark_scooters.hpp"
#include "ark_transactions.hpp"
#include "fonts/fonts.hpp"
#include "hardware/display.hpp"
#include "hardware/gps.hpp"
#include "hardware/qr_code.hpp"
#include "helpers.hpp"
#include "timer.hpp"

/*******************************************************************************
 * mbed TLS Library for SHA256 function
 *
 * https://techtutorialsx.com/2018/05/10/esp32-arduino-mbed-tls-using-the-sha-256-algorithm/#more-25918
 * support for sha256
 *
 * hash generator to check results of library.
 * https://passwordsgenerator.net/sha256-hash-generator/
 ******************************************************************************/
#include "mbedtls/md.h"

mbedtls_md_context_t ctx;
mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;  // select SHA256 algorithm

// Global Rental State
State_enum state = STATE_0;

// state machine vars
char _stateAddress[35] = {0};
char _stateRateStr[20] = {0};
uint64_t _stateRate64 = 0ULL;

/*******************************************************************************
 * Set state machine variables.
 ******************************************************************************/
void setStateMachineVars(const char *address, const char *rateStr,
                         uint64_t rate64) {
  strcpy(_stateAddress, address);
  strcpy(_stateRateStr, rateStr);
  _stateRate64 = rate64;
}

/*******************************************************************************
 * Set the current state variable and print to console.
 ******************************************************************************/
void setState(state_enum_t newState) {
  state = newState;
  Serial.printf("\nState: %s", state);
}

/*******************************************************************************
 * Verify the WiFi Connection.
 ******************************************************************************/
void verifyWifi() {
  if (!WiFi_status) {
    scooterRental.rentalStatus = "Broken";
    setState(STATE_0);
  } else {
    setState(STATE_1);
  }
}

/*******************************************************************************
 * Verify the Mqtt Connection.
 ******************************************************************************/
void verifyMqtt() { !MQTT_status ? setState(STATE_1) : setState(STATE_2); }

/*******************************************************************************
 * Verify the Bridgechain Network Connection.
 ******************************************************************************/
void verifyBridgechainConnection() {
  !ARK_status ? setState(STATE_2) : setState(STATE_3);
}

/*******************************************************************************
 * State 3
 *
 * Transitions to state 4 once GPS gets a satellite lock.
 *
 * Transition Actions:
 * - rentalStatus = "Available"
 * - generate and display QR code
 ******************************************************************************/
void setScooterAvailable() {
  if (GPS_status) {  // wait for GPS fix

    // this is pseudorandom when the wifi or bluetooth does not have a
    // connection. It can be considered "random" when the radios have a
    // connection arduino random function is overloaded on to esp_random();
    // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/system.html

    // generate 32 bit random number with a lower and upper bound using
    // ESP32 RNG.
    uint32_t esprandom = (esp_random());

    // QRcode Version = 10 with ECC=2 gives 211 Alphanumeric characters or
    // 151 bytes(any characters)
    char QRcodeText[256 + 1];

    strcpy(QRcodeText, "rad:");
    strcat(QRcodeText, _stateAddress);

    // start sha256
    // use this to check result of SHA256
    // https://passwordsgenerator.net/sha256-hash-generator/
    // http://www.fileformat.info/tool/hash.htm

    char SHApayload[10 + 1];  // max number is 4294967295

    // use this instead for unsigned conversion
    utoa(esprandom, SHApayload, 10);

    // holds length of payload
    const size_t payloadLength = strlen(SHApayload);

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *)SHApayload, payloadLength);
    mbedtls_md_finish(&ctx,
                      shaResult);  // shaResult is global variable of type bytes
    mbedtls_md_free(&ctx);

    // display the value to be hashed
    Serial.printf("\nvalue to be Hashed: %s", SHApayload);

    // convert the SHAresult which is an array of bytes into an array of
    // characters so we can send to terminal display
    char shaResult_char[64 + 1];
    shaResult_char[0] = '\0';
    for (int i = 0; i < sizeof(shaResult); i++) {
      char str[3];
      sprintf(str, "%02x", (int)shaResult[i]);
      strcat(shaResult_char, str);
    }

    // display the resulting SHA256
    Serial.printf("QRcode SHA256: %s", shaResult_char);
    // end sha256

    strcat(QRcodeText, "?hash=");

    // append hash to QRcode string
    strcat(QRcodeText, shaResult_char);
    // stash hash away for use later in rental start transaction handler
    strcpy(QRcodeHash, shaResult_char);

    strcat(QRcodeText, "&rate=");
    strcat(QRcodeText, _stateRateStr);

    // record GPS coordinates of the Rental Start
    scooterRental.startLatitude =
        convertDegMinToDecDeg_lat(GPS.latitude, GPS.lat);
    scooterRental.startLongitude =
        convertDegMinToDecDeg_lon(GPS.longitude, GPS.lon);

    char QRLatitude[13];
    // create string with 6 decimal point.
    snprintf(&QRLatitude[0], 13, "%.6f", scooterRental.QRLatitude);

    char QRLongitude[13];
    // create string with 6 decimal point.
    snprintf(&QRLongitude[0], 13, "%.6f", scooterRental.QRLongitude);

    strcat(QRcodeText, "&lat=");
    strcat(QRcodeText, QRLatitude);

    strcat(QRcodeText, "&lon=");
    strcat(QRcodeText, QRLongitude);

    Serial.printf("\nQR text: %s", QRcodeText);
    displayQRcode(QRcodeText);

    // reset transaction search counter
    previousUpdateTime_RentalStartSearch = millis();

    scooterRental.rentalStatus = "Available";

    setState(STATE_4);
  } else {
    scooterRental.rentalStatus = "Broken";
    setState(STATE_3);
  }
}

/*******************************************************************************
 * State 4
 *
 * Transitions to state 5 once valid RentalStart transaction is forged.
 *
 * Transition Actions:
 * - unlock scooter
 * - rentalStatus = "Rented"
 * - Start Ride Timer
 * - Initialize display with speedometer and ride timer
 ******************************************************************************/
void setScooterRented() {
  if (!GPS_status) {
    // Gps disconnected
    setState(STATE_3);
    return;
  }

  if (!search_RentalStartTx()) {
    // Rental Start Tx not found
    setState(STATE_4);
  } else {
    // Rental Start Tx found
    Serial.println("Start Ride Timer");
    // record Unix timestamp of the Rental start
    scooterRental.startTime = time(nullptr);

    // We are using the ms timer for the ride timer.
    // This id probably redundant.
    // We could use the previous unix timer
    rideTime_start_ms = millis();

    // record GPS coordinates of the Rental Start
    scooterRental.startLatitude =
        convertDegMinToDecDeg_lat(GPS.latitude, GPS.lat);
    scooterRental.startLongitude =
        convertDegMinToDecDeg_lon(GPS.longitude, GPS.lon);

    // calculate the ride length = received payment / Rental
    // rate(RAD/seconds)
    uint64_t rideTime_length_sec = scooterRental.payment_Uint64 / _stateRate64;

    rideTime_length_ms = rideTime_length_sec * 1000;  // convert to ms
    Serial.printf("ride time length: %lu", rideTime_length_ms);

    // this is used by the countdown timer to refresh the display only
    // once each second.
    remainingRentalTime_previous_s = rideTime_length_sec;

    clearMainScreen();

    tft.setFont(&Lato_Medium_36);
    // http://www.barth-dev.de/online/rgb565-color-picker/
    tft.setTextColor(SpeedGreen);
    tft.setCursor(75, 150);
    tft.print("km/h");

    previousSpeed = 0;
    // get current speed with full precision
    updateSpeedometer(GPS.speed * 1.852);

    unlockScooter();

    scooterRental.rentalStatus = "Rented";

    setState(STATE_5);
  }
}

/*******************************************************************************
 * State 5
 *
 * Transitions to state 6 once ride timer expires.
 *
 * Speedometer and Ride timer are regularly updated on the display.
 *
 * Transition Actions:
 * - lock scooter
 * - rentalStatus = "Available"
 * - Send RentalFinish blockchan transaction
 *   (code currently does not check WiFi connection prior to sending)
 * - Initialize display with speedometer and ride timer
 *
 * In this state the WiFi, MQTT, Ark, and GPS connections are ignored.
 ******************************************************************************/
void setScooterSessionActive() {
  // rider is using scooter, wait for timer to expire,
  // then lock scooter and send rental finish and go back to beginning
  if (millis() - rideTime_start_ms > rideTime_length_ms) {
    // timer has expired
    // use difftime
    // http://www.cplusplus.com/reference/ctime/difftime/

    // record Unix timestamp of the Rental Finish
    scooterRental.endTime = time(nullptr);

    // record GPS coordinates of the Rental Finish
    scooterRental.endLatitude =
        convertDegMinToDecDeg_lat(GPS.latitude, GPS.lat);
    scooterRental.endLongitude =
        convertDegMinToDecDeg_lon(GPS.longitude, GPS.lon);

    // We need to retrieve Wallet Nonce before you send a transaction
    getWallet();

    SendTransaction_RentalFinish();

    Serial.println("=================================");
    Serial.printf("Rental Structure: %s\n%s\n%" PRIu64 "\n%s",
                  scooterRental.senderAddress, scooterRental.payment,
                  scooterRental.payment_Uint64, scooterRental.rentalRate);
    // this prints out only 6 decimal places.  It has 8 decimals
    Serial.println(scooterRental.startLatitude, 6);
    Serial.println(scooterRental.startLongitude, 6);
    Serial.println(scooterRental.endLatitude, 6);
    Serial.println(scooterRental.endLongitude, 6);
    Serial.println(scooterRental.vendorField);
    Serial.println("=================================");

    scooterRental.rentalStatus = "Available";

    setState(STATE_6);
  } else {
    // timer has not expired
    // no speed is shown initially if there is no change.
    // get current speed with full precision
    updateSpeedometer(GPS.speed * 1.852);
    updateCountdownTimer();

    setState(STATE_5);
  }
}

/*******************************************************************************
 * Mealy Finite State Machine
 * The state machine logic is executed once each cycle of the "main" loop.
 ******************************************************************************/
void StateMachine() {
  switch (state) {
    case STATE_0: {
      // Initial state after microcontroller powers up.
      // Transitions to:
      // - state 1 once WiFi is connected
      verifyWifi();
      break;
    }

    case STATE_1: {
      // Transitions to state 2 once connected to MQTT broker
      // Return to:
      // - state 0 if WiFi disconnects
      verifyWifi();
      verifyMqtt();
      break;
    }

    case STATE_2: {
      // Transitions to state 3 once connected to Ark Node
      // Return to:
      // - state 0 if WiFi disconnects
      // - state 1 if MQTT disconnects
      verifyWifi();
      verifyMqtt();
      verifyBridgechainConnection();
      break;
    }

    case STATE_3: {
      // Transitions to state 4 once GPS gets a satellite lock.
      // Transition Actions:
      //  - rentalStatus = "Available"
      //  - generate and display QR code
      //
      // Return to:
      // - state 0 if WiFi disconnects
      // - state 1 if MQTT disconnects
      // - state 2 if Ark network disconnects
      verifyWifi();
      verifyMqtt();
      verifyBridgechainConnection();
      setScooterAvailable();
      break;
    }

    case STATE_4: {
      // Transitions to state 5 once valid RentalStart blockchain transaction is
      // received Transition Actions:
      //  - unlock scooter
      //  - rentalStatus = "Rented"
      //  - Start Ride Timer
      //  - Initialize display with speedometer and ride timer
      //
      // Return to:
      // - state 0 if WiFi disconnects
      // - state 1 if MQTT disconnects
      // - state 2 if Ark network disconnects
      // - state 3 if GPS loses lock
      DisplayArkBitmap();
      verifyWifi();
      verifyMqtt();
      verifyBridgechainConnection();
      setScooterRented();
      break;
    }

    case STATE_5: {
      // Transitions to state 6 once ride timer expires.  Speedometer and Ride
      // timer are regularly updated on the display Transition Actions:
      //  - lock scooter
      //  - rentalStatus = "Available"
      //  - Send RentalFinish blockchan transaction (code currently does not
      //  check to make sure a WiFi connection is available prior to sending).
      //  - Initialize display with speedometer and ride timer
      //
      // In this state the WiFi, MQTT, Ark, and GPS connections are ignored.
      setScooterSessionActive();
      break;
    }

    case STATE_6: {
      // Currently nothing happens in this state.
      // Go immediately back to state 3.
      setState(STATE_3);
      break;
    }
  }
}

/*******************************************************************************
 * unlock the scooter
 ******************************************************************************/
void unlockScooter() { Serial.printf("\nScooter Unlocked!\n"); }
