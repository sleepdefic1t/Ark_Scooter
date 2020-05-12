
#ifndef ARK_SCOOTERS_UTILS_TIMER_HPP
#define ARK_SCOOTERS_UTILS_TIMER_HPP

#include <cstdint>

extern uint32_t rideTime_start_ms;
extern uint32_t rideTime_length_ms;

extern uint32_t remainingRentalTime_previous_s;

// time variables use for clock on the display
// this is used if you want to update clock every minute
extern int prevDisplayMinute;

/*******************************************************************************
 * Update Intervals for various algorithms
 ******************************************************************************/
// Frequency at which the MQTT packets are published
extern uint32_t UpdateInterval_MQTT_Publish;
extern uint32_t previousUpdateTime_MQTT_Publish;

// Frequency at which the battery level is updated on the screen
extern uint32_t UpdateInterval_Battery;
extern uint32_t previousUpdateTime_Battery;

// Frequency at which the WiFi Receive Signal Level is updated on the screen
extern uint32_t UpdateInterval_RSSI;
extern uint32_t previousUpdateTime_RSSI;

// Frequency at which the Network polls looking for a rental start transaction
extern uint32_t UpdateInterval_RentalStartSearch;
extern uint32_t previousUpdateTime_RentalStartSearch;

// Frequency at which the Speed and # GPS Satellites are updated on the screen
extern uint32_t UpdateInterval_GPS;
extern uint32_t previousUpdateTime_GPS;

#endif  // ARK_SCOOTERS_UTILS_TIMER_HPP
