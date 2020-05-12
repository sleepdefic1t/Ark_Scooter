
#include "configuration.hpp"

#include "ark_libs.hpp"

// BridgeChain Network Structure Model. These are defined in secrets.hpp
Network BridgechainNetwork;

// Load the Custom Network Configuration
Configuration cfg;

/*******************************************************************************
 * Set configuration.
 ******************************************************************************/
void setConfiguration(const char *nethash, uint8_t slip44, uint8_t wif,
                      uint8_t version, const char *epoch) {
  BridgechainNetwork = {nethash, slip44, wif, version, epoch};

  cfg = Configuration(BridgechainNetwork);
}
