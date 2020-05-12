/*******************************************************************************
 * This file contains functions that interact with Ark client C++ API
 ******************************************************************************/

#ifndef ARK_SCOOTERS_TRANSACTIONS_ARK_TRANSACTIONS_HPP
#define ARK_SCOOTERS_TRANSACTIONS_ARK_TRANSACTIONS_HPP

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
bool checkArkNodeStatus();

/*******************************************************************************
 * This routine retrieves the current nonce and the balance for the wallet
 ******************************************************************************/
void getWallet();

/*******************************************************************************
 * This routine polls the ARK Node API for the RentalStart custom transaction.
 * It polls once every 8 seconds When polling the API we set the limit.
 ******************************************************************************/
int search_RentalStartTx();

/*******************************************************************************
 * This routine retrieves 1 received transaction in wallet if available.
 ******************************************************************************/
int GetReceivedTransaction(const char *const address, int page, const char *&id,
                           const char *&amount, const char *&senderAddress,
                           const char *&senderPublicKey,
                           const char *&vendorField);

/*******************************************************************************
 * This routine will search through all the received transactions of ArkAddress
 * wallet starting from the oldest.
 *
 * The routine returns the page number of the most recent transaction.
 ******************************************************************************/
int getMostRecentReceivedTransaction(int page = 1);

/*******************************************************************************
 * This routine retrieves 1 RentalStart transaction if available in wallet
 * Returns '0' if no transaction exists or if other transaction type exists
 * Pass wallet address and page to function
 ******************************************************************************/
int GetTransaction_RentalStart(
    const char *const address, int page, const char *&id, const char *&amount,
    const char *&senderAddress, const char *&senderPublicKey,
    const char *&vendorField, const char *&asset_gps_latitude,
    const char *&asset_gps_longitude, const char *&asset_sessionId,
    const char *&asset_rate);

/*******************************************************************************
 * Send a Rental Finish Custom BridgeChain transaction
 ******************************************************************************/
void SendTransaction_RentalFinish();

/*******************************************************************************
 * Send a BridgeChain transaction, tailored for a custom network.
 ******************************************************************************/
void sendBridgechainTransaction();

#endif  // ARK_SCOOTERS_TRANSACTIONS_ARK_TRANSACTIONS_HPP
