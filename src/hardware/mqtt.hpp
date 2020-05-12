
#ifndef ARK_SCOOTERS_HARDWARE_MQTT_HPP
#define ARK_SCOOTERS_HARDWARE_MQTT_HPP

#include "gps.hpp"

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
 *NodeRed client
 ******************************************************************************/
struct MQTTpacket {
  const char *status;
  int battery;
  int fix;
  int satellites;
  float latitude;
  float longitude;
  float speedKPH;
  char walletBalance[20];
  char signature[145];  // <- Ecdsa max signature size + '\0' (2 * 72 + 1)
};

extern struct MQTTpacket NodeRedMQTTpacket;

/*******************************************************************************
 * Create the Mqtt WiFi Client.
 ******************************************************************************/
void configureMqttClient(EspMQTTClient client, const char *mqttBaseTopic);

/*******************************************************************************
 * send structure to NodeRed MQTT broker
 ******************************************************************************/
void send_MQTTpacket(int batteryPercent, const char *rentalStatus,
                     Adafruit_GPS gps, const char *walletBalance,
                     const char *passphrase, const char *mqttBaseTopic,
                     EspMQTTClient client);

/*******************************************************************************
 * Update status bar with MQTT connection status.
 * Display only updates on connection status change
 ******************************************************************************/
void UpdateMQTTConnectionStatus(bool status, EspMQTTClient client);

#endif  // ARK_SCOOTERS_HARDWARE_MQTT_HPP
