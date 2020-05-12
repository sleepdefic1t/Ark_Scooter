
#include "mqtt.hpp"

#include "ark_libs.hpp"
#include "display.hpp"
#include "helpers.hpp"
#include "timer.hpp"

/*******************************************************************************
 * EspMQTTClient Library by @plapointe6 Version 1.8.0
 * WiFi and MQTT connection handler for ESP32
 * This library does a nice job of encapsulating the handling of WiFi and MQTT
 * connections. You just need to provide your credentials and it will manage the
 * connection and reconnections to the Wifi and MQTT networks. EspMQTTClient is
 * a wrapper around the MQTT PubSubClient Library Version 2.7 by @knolleary
 *
 * The MQTT packets are larger then the allowed for the default setting of the
 * libary. You need to update this line in PubSubClient.h. Setting it here does
 * nothing. If you update this library you will need to update this setting as
 * it will be overwritten.
 ******************************************************************************/
#include "EspMQTTClient.h"

/*******************************************************************************
 * This is the data packet that is sent to the CloudMQTT broker and then read by
 * NodeRed client
 ******************************************************************************/
struct MQTTpacket NodeRedMQTTpacket;

/*******************************************************************************
 * Create the Mqtt WiFi Client.
 ******************************************************************************/
void configureMqttClient(EspMQTTClient client, const char *mqttBaseTopic) {
  // Optional Features of EspMQTTClient
  // Enable debugging messages sent to serial output
  client.enableDebuggingMessages();

#ifdef ENABLE_WIRELESS_UPDATE
  // Enable the web updater.
  // User and password default to values of MQTTUsername and MQTTPassword.
  // These can be overwritten with enableHTTPWebUpdater("user", "password").
  client.enableHTTPWebUpdater();
#endif

  // You can activate the retain flag by setting the third parameter to true
  client.enableLastWillMessage(mqttBaseTopic, "{\"status\":\"Broken\"}");
}

/*******************************************************************************
 * Fill in the data structure to be sent via MQTT
 *
 * const char* status;
 * int battery;
 * int fix;
 * int satellites;
 * float latitude;
 * float longitude;
 * float speedKPH;
 * char walletBalance[64];
 ******************************************************************************/
void build_MQTTpacket(int batteryPercent, const char *rentalStatus,
                      Adafruit_GPS gps, const char *walletBalance) {
  NodeRedMQTTpacket.battery = batteryPercent;
  strcpy(NodeRedMQTTpacket.walletBalance, walletBalance);

  NodeRedMQTTpacket.status = rentalStatus;

  NodeRedMQTTpacket.fix = int(gps.fix);
  if (NodeRedMQTTpacket.fix) {
    NodeRedMQTTpacket.satellites = gps.satellites;   // number of satellites
    NodeRedMQTTpacket.speedKPH = GPS.speed * 1.852;  // convert knots to kph

    // we need to do some formatting of the GPS signal so it is suitable for
    // mapping software on Thingsboard
    NodeRedMQTTpacket.latitude = 53.53583908;     // default location
    NodeRedMQTTpacket.longitude = -113.27674103;  // default location

    NodeRedMQTTpacket.latitude =
        convertDegMinToDecDeg_lat(gps.latitude, gps.lat);
    NodeRedMQTTpacket.longitude =
        convertDegMinToDecDeg_lon(gps.longitude, gps.lon);
  } else {
    // we do not have a GPS fix. What should the GPS location be?
    // NodeRedMQTTpacket.status = "Broken";
    NodeRedMQTTpacket.latitude = 53.53583908;     // default location
    NodeRedMQTTpacket.longitude = -113.27674103;  // default location
    NodeRedMQTTpacket.satellites = 0;             // number of satellites
    NodeRedMQTTpacket.speedKPH = 0;               // speed
  }
}

/*******************************************************************************
 * send structure to NodeRed MQTT broker
 ******************************************************************************/
void send_MQTTpacket(int batteryPercent, const char *rentalStatus,
                     Adafruit_GPS gps, const char *walletBalance,
                     const char *passphrase, const char *mqttBaseTopic,
                     EspMQTTClient client) {
  if (millis() - previousUpdateTime_MQTT_Publish >
      UpdateInterval_MQTT_Publish) {
    previousUpdateTime_MQTT_Publish += UpdateInterval_MQTT_Publish;

    if (client.isMqttConnected()) {
      build_MQTTpacket(batteryPercent, rentalStatus, gps, walletBalance);

      // NOTE!  I think sprintf() is better to use here. update when you have a
      // chance
      // example:
      // {"status":"Rented","fix":1,"lat":53.53849358,"lon":-113.27589669,"speed":0.74,"sat":5,"bal":99990386752,"bat":96}
      String buf;
      buf += F("{");
      buf += F("\"status\":");
      buf += F("\"");
      buf += String(NodeRedMQTTpacket.status);
      buf += F("\"");
      buf += F(",\"fix\":");
      buf += String(NodeRedMQTTpacket.fix);
      buf += F(",\"lat\":");
      // add noise to gps signal.  use 8 decimal point precision.
      buf += String(NodeRedMQTTpacket.latitude + 0.0032, 8);
      buf += F(",\"lon\":");
      buf += String(NodeRedMQTTpacket.longitude + 0.00221, 8);
      buf += F(",\"speed\":");
      buf += String(NodeRedMQTTpacket.speedKPH);
      buf += F(",\"sat\":");
      buf += String(NodeRedMQTTpacket.satellites);
      buf += F(",\"bal\":");
      buf += String(NodeRedMQTTpacket.walletBalance);
      buf += F(",\"bat\":");
      buf += String(NodeRedMQTTpacket.battery);

      // These are pointers! They are not copying
      // get string without leading {
      const char *msg = buf.substring(1).c_str();

      char msgbackup[500 + 1];
      strcpy(msgbackup, msg);

      // sign the packet using Private Key
      Message message;
      message.sign(msg, passphrase);
      const auto signatureString = BytesToHex(message.signature);

      buf += F(",\"sig\":");
      buf += F("\"");
      buf += signatureString.c_str();  // append the signature
      buf += F("\"");
      buf += F("}");

      printf("\n\nSignature from Signed Message: %s\n",
             signatureString.c_str());
      const bool isValid = message.verify();  // verify the signature
      printf("\nMessage Signature is valid: %s\n\n",
             isValid ? "true" : "false");
      Serial.printf("\nmessage that was signed: %s", msgbackup);

      Serial.printf("\nsend_MQTTpacket: ");
      Serial.println(buf);

      client.publish(mqttBaseTopic, buf.c_str());
    }
  }
}

/*******************************************************************************
 * Update status bar with MQTT connection status.
 * Display only updates on connection status change
 ******************************************************************************/
void UpdateMQTTConnectionStatus(bool status, EspMQTTClient client) {
  if (client.isMqttConnected()) {
    if (!status) {
      // x,y,radius,color  //MQTT Status
      tft.fillCircle(130, 301 - 6, 6, GREEN);
      status = true;
    }
  } else {
    if (status) {
      // x,y,radius,color  //MQTT Status
      tft.fillCircle(130, 301 - 6, 6, RED);
      status = false;
    }
  }
}
