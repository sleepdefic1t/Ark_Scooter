
#include "helpers.hpp"

#include <math.h> /* fmod */

/*******************************************************************************
 * converts latitude from Adafruit degree-minute format to decimal-degrees.
 *
 * http://arduinodev.woofex.net/2013/02/06/adafruit_gps_forma/
 *
 * measuring accuracy of GPS:
 * https://gis.stackexchange.com/questions/8650/measuring-accuracy-of-latitude-and-longitude/8674#8674
 ******************************************************************************/
double convertDegMinToDecDeg_lat(float degMin, char gpsLatChar) {
  double min = 0.0;
  double decDeg = 0.0;

  degMin = (int)(degMin / 100);

  decDeg = degMin + (min / 60);

  if (gpsLatChar == 'S') {
    decDeg = (0 - decDeg);
  }

  return decDeg;
}

/*******************************************************************************
 * converts longitude from Adafruit degree-minute format to decimal-degrees.
 *
 * https://gis.stackexchange.com/questions/8650/measuring-accuracy-of-latitude-and-longitude/8674#8674
 ******************************************************************************/
double convertDegMinToDecDeg_lon(float degMin, char gpsLonChar) {
  double min = 0.0;
  double decDeg = 0.0;

  min = fmod((double)degMin, 100.0);
  degMin = (int)(degMin / 100);
  decDeg = degMin + (min / 60);

  if (gpsLonChar == 'W') {
    decDeg = (0 - decDeg);
  }
  return decDeg;
}
