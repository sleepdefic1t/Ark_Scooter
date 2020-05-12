/*******************************************************************************
 * This file contains functions that interact with Ark client C++ API
 ******************************************************************************/

#ifndef ARK_SCOOTERS_TRANSACTIONS_ARK_TRANSACTIONS_HPP
#define ARK_SCOOTERS_TRANSACTIONS_ARK_TRANSACTIONS_HPP

#include <arkClient.h>

#include <cstdint>

/*******************************************************************************
 * Set transaction variables.
 ******************************************************************************/
void setTransactionVars(const char *address, const char *passphrase,
                        uint64_t fee);

/*******************************************************************************
 * The following method can be used to get the Status of a Node.
 * This is equivalent to calling '167.114.29.49:4003/api/v2/node/status'
 *
 * https://arduinojson.org/v6/api/jsondocument/
 * https://arduinojson.org/v6/assistant/
 ********************************************************************************/
bool checkArkNodeStatus(Ark::Client::Connection<Ark::Client::Api> &connection);

/*******************************************************************************
 * This routine retrieves the current nonce and the balance for the wallet
 ******************************************************************************/
bool getWallet(Ark::Client::Connection<Ark::Client::Api> &connection);

/*******************************************************************************
 * This routine polls the ARK Node API for the RentalStart custom transaction.
 * It polls once every 8 seconds When polling the API we set the limit.
 ******************************************************************************/
int search_RentalStartTx(Ark::Client::Connection<Ark::Client::Api> &connection);

/*******************************************************************************
 * This routine retrieves 1 received transaction in wallet if available.
 ******************************************************************************/
int GetReceivedTransaction(
    Ark::Client::Connection<Ark::Client::Api> &connection,
    const char *const address, int page, const char *&id, const char *&amount,
    const char *&senderAddress, const char *&senderPublicKey,
    const char *&vendorField);

/*******************************************************************************
 * This routine will search through all the received transactions of ArkAddress
 * wallet starting from the oldest.
 *
 * The routine returns the page number of the most recent transaction.
 ******************************************************************************/
int getMostRecentReceivedTransaction(
    Ark::Client::Connection<Ark::Client::Api> &connection, int page = 1);

/*******************************************************************************
 * This routine retrieves 1 RentalStart transaction if available in wallet
 * Returns '0' if no transaction exists or if other transaction type exists
 * Pass wallet address and page to function
 ******************************************************************************/
int GetTransaction_RentalStart(
    Ark::Client::Connection<Ark::Client::Api> &connection,
    const char *const address, int page, const char *&id, const char *&amount,
    const char *&senderAddress, const char *&senderPublicKey,
    const char *&vendorField, const char *&asset_gps_latitude,
    const char *&asset_gps_longitude, const char *&asset_sessionId,
    const char *&asset_rate);

/*******************************************************************************
 * Send a Rental Finish Custom BridgeChain transaction
 ******************************************************************************/
void SendTransaction_RentalFinish(
    Ark::Client::Connection<Ark::Client::Api> &connection);

/*******************************************************************************
 * Send a BridgeChain transaction, tailored for a custom network.
 ******************************************************************************/
void sendBridgechainTransaction(
    Ark::Client::Connection<Ark::Client::Api> &connection);

#endif  // ARK_SCOOTERS_TRANSACTIONS_ARK_TRANSACTIONS_HPP
