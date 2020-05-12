
#ifndef ARK_SCOOTERS_ARK_SCOOTERS_H
#define ARK_SCOOTERS_ARK_SCOOTERS_H

#include "ark_transactions.hpp"
#include "configuration.hpp"
#include "hardware/display.hpp"
#include "hardware/gps.hpp"
#include "hardware/mqtt.hpp"
#include "rental.hpp"
#include "state_machine.hpp"

/*******************************************************************************
 * Misc I/O Definitions
 ******************************************************************************/
extern const int LED_PIN;  // LED integrated on Adafruit HUZZAH32 module

/*******************************************************************************
 * Various Global Variables
 ******************************************************************************/
// used to detect first run after power up
extern bool initialConnectionEstablished_Flag;

extern bool ARK_status;
extern bool GPS_status;
extern bool MQTT_status;
extern bool WiFi_status;

/*******************************************************************************
 * Set scooter variables.
 ******************************************************************************/
void setScooterVars(const char *rateStr, uint64_t rate64, uint8_t timeZone,
                    uint8_t dst, EspMQTTClient *client);

/*******************************************************************************
 * This function is called once WiFi and MQTT connections are complete.
 *
 * This is a callback function from the Mqtt Library.
 *******************************************************************************/
void onConnectionEstablished();

#endif  // ARK_SCOOTERS_ARK_SCOOTERS_HPP
