
#ifndef ARK_SCOOTERS_CONFIGURATION_H
#define ARK_SCOOTERS_CONFIGURATION_H

#include "ark_libs.hpp"

// BridgeChain Network Structure Model. These are defined in secrets.hpp
extern Network BridgechainNetwork;

// Load the Custom Network Configuration
extern Configuration cfg;

// create ARK blockchain connection
extern Ark::Client::Connection<Ark::Client::Api> connection;

/*******************************************************************************
 * Set configuration.
 ******************************************************************************/
void setConfiguration(const char *nethash, uint8_t slip44, uint8_t wif,
                      uint8_t version, const char *epoch, const char *peer,
                      int port);

#endif  // ARK_SCOOTERS_CONFIGURATION_H
