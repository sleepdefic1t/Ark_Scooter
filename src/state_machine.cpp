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

#include <arkClient.h>

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
static void setState(state_enum_t newState) {
  state = newState;
  Serial.printf("\nEntering state: %d\n", state);
  if (state == STATE_0) {
    delay(1000);
  }
}

/*******************************************************************************
 * Generates the QRcode and displays on TFT display
 ******************************************************************************/
static void GenerateDisplay_QRcode() {
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
}

/*******************************************************************************
 * Mealy Finite State Machine
 * The state machine logic is executed once each cycle of the "main" loop.
 ******************************************************************************/
void StateMachine(Ark::Client::Connection<Ark::Client::Api> &connection) {
  switch (state) {
    case STATE_0: {
      // Initial state after microcontroller powers up.
      // Transitions to:
      // - state 1 once WiFi is connected
      if (WiFi_status) {
        Serial.print("\nObtained WiFi connection.\n");
        setState(STATE_1);
      } else {
        scooterRental.rentalStatus = "Broken";
        setState(STATE_0);
      }
      break;
    }

    case STATE_1: {
      // Transitions to state 2 once connected to MQTT broker
      // Return to:
      // - state 0 if WiFi disconnects
      if (!WiFi_status) {  // check for WiFi disconnect
        state = STATE_0;
      } else if (MQTT_status) {  // wait for MQTT connect
        state = STATE_2;
        Serial.printf("\nObtained MQTT connection.\n");
        setState(STATE_2);
      } else {
        scooterRental.rentalStatus = "Broken";
        state = STATE_1;
      }
      break;
    }

    case STATE_2: {
      // Transitions to state 3 once connected to Ark Node
      // Return to:
      // - state 0 if WiFi disconnects
      // - state 1 if MQTT disconnects
      if (!WiFi_status) {  // check for WiFi disconnect
        state = STATE_0;
      } else if (!MQTT_status) {  // check for MQTT disconnect
        state = STATE_1;
      } else if (ARK_status) {  // wait for ARK network connect
        state = STATE_3;
        Serial.print("\nObtained Radians connection.\n");
        setState(STATE_3);
      } else {
        scooterRental.rentalStatus = "Broken";
        setState(STATE_2);
      }
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
      if (!WiFi_status) {  // check for WiFi disconnect
        state = STATE_0;
      } else if (!MQTT_status) {  // check for MQTT disconnect
        state = STATE_1;
      } else if (!ARK_status) {  // check for ARK network disconnect
        state = STATE_2;
      } else if (GPS_status) {  // wait for GPS fix

        GenerateDisplay_QRcode();

        // reset transaction search counter
        previousUpdateTime_RentalStartSearch = millis();

        scooterRental.rentalStatus = "Available";
        Serial.print("\nObtained GPS connection.\n");
        setState(STATE_4);
      } else {
        scooterRental.rentalStatus = "Broken";
        setState(STATE_3);
      }
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
      if (!WiFi_status) {  // check for WiFi disconnect
        DisplayArkBitmap();
        setState(STATE_0);
      } else if (!MQTT_status) {  // check for MQTT disconnect
        DisplayArkBitmap();
        setState(STATE_1);
      } else if (!ARK_status) {  // check for ARK network disconnect
        DisplayArkBitmap();
        setState(STATE_2);
      } else if (!GPS_status) {  // check for GPS network disconnect
        DisplayArkBitmap();
        setState(STATE_3);
      } else {  // we are looking for a Rental Start Tx
        if (search_RentalStartTx(connection)) {
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
          uint64_t rideTime_length_sec =
              scooterRental.payment_Uint64 / _stateRate64;

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

          Serial.println("\nRental Started.\n");
          setState(STATE_5);
          break;
        } else {
          setState(STATE_4);
          break;
        }
      }
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
      // wait for timer to expire and then lock scooter and send rental finish
      // and go back to beginning
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
        if (!getWallet(connection)) {
          return;
        }

        SendTransaction_RentalFinish(connection);

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
        // proceed to next state
      } else {
        // timer has not expired
        // no speed is shown initially if there is no change
        updateSpeedometer(GPS.speed * 1.852);
        updateCountdownTimer();
        setState(STATE_5);
      }
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
