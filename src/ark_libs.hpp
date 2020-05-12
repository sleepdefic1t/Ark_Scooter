

#ifndef ARK_SCOOTERS_ARK_LIBS_HPP
#define ARK_SCOOTERS_ARK_LIBS_HPP

/*******************************************************************************
 * Ark Crypto Library (version 1.0.0)
 *
 * ================
 * Version 1.1.0 is available however I have not yet tested with it
 * ===============
 * https://github.com/ArkEcosystem/Cpp-Crypto
 *
 * If this Repo was Cloned from github,
 * run the 'ARDUINO_IDE.sh' script first.
 * It's in the 'extras/' folder and extends compatability to the Arduino IDE.
 *
 * Bip66 Library (version 0.3.2)
 * https://github.com/sleepdefic1t/bip66
 ******************************************************************************/
#include <arkCrypto.h>

// This is a helper header that includes all the Misc ARK C++ Crypto headers
// required for this sketch
#include "arkCrypto_esp32.h"
#include "transactions/builders/radians/radians.hpp"

// Namespaces
using namespace Ark::Crypto;
using namespace Ark::Crypto::identities;
using namespace Ark::Crypto::transactions;

/*******************************************************************************
 * Ark Client Library (version 1.4.0)
 * ================
 * Version 1.4.1 is available however I have not yet tested with it ===========
 * https://github.com/ArkEcosystem/cpp-client
 *
 * https://docs.ark.io/iot/#which-sdk-supports-iot
 * https://docs.ark.io/tutorials/iot/storing-data-on-the-blockchain.html#step-1-project-setup
 * https://docs.ark.io/tutorials/iot/reacting-to-data-on-the-blockchain.html#step-1-project-setup
 ******************************************************************************/
#include <arkClient.h>

#endif  // ARK_SCOOTERS_ARK_LIBS_HPP
