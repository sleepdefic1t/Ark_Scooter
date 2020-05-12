
#include "timer.hpp"

#include <Arduino.h>
#include <cstdint>

/*******************************************************************************
 * Time Library
 * required for internal clock to synchronize with NTP server.
 ******************************************************************************/
#include "time.h"

uint32_t rideTime_start_ms = 0;
uint32_t rideTime_length_ms = 0;

uint32_t remainingRentalTime_previous_s = 0;

time_t prevDisplayTime = 0;

int prevDisplayMinute = 0;

/*******************************************************************************
 * Update Intervals for various algorithms
 ******************************************************************************/

uint32_t UpdateInterval_MQTT_Publish = 15000;  // 15 seconds
uint32_t previousUpdateTime_MQTT_Publish = millis();

uint32_t UpdateInterval_Battery = 7000;  // 7 seconds
uint32_t previousUpdateTime_Battery = millis();

uint32_t UpdateInterval_RSSI = 5000;  // 5 seconds
uint32_t previousUpdateTime_RSSI = millis();

uint32_t UpdateInterval_RentalStartSearch = 8000;  // 8 seconds
uint32_t previousUpdateTime_RentalStartSearch = millis();

uint32_t UpdateInterval_GPS = 5000;
uint32_t previousUpdateTime_GPS = millis();
