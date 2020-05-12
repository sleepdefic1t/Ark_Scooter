
#ifndef ARK_SCOOTERS_UTILS_HELPERS_HPP
#define ARK_SCOOTERS_UTILS_HELPERS_HPP

/*******************************************************************************
 * converts latitude from Adafruit degree-minute format to decimal-degrees.
 ******************************************************************************/
double convertDegMinToDecDeg_lat(float degMin, char gpsLatChar);

/*******************************************************************************
 * converts longitude from Adafruit degree-minute format to decimal-degrees.
 ******************************************************************************/
double convertDegMinToDecDeg_lon(float degMin, char gpsLonChar);

#endif  // ARK_SCOOTERS_UTILS_HELPERS_HPP
