/********************************************************************************
  This file contains functions that interact with Ark client C++ API
********************************************************************************/

#include "ark_transactions.hpp"

#include <Arduino.h>
#include <arkClient.h>
#include <stdlib.h>

#include <string>

#include "configuration.hpp"
#include "hardware/eeprom.hpp"
#include "hardware/qr_code.hpp"
#include "rental.hpp"
#include "timer.hpp"

// transaction vars
char _txAddress[35] = {0};
char _passphrase[128] = {0};
uint64_t _fee = 0ULL;

/*******************************************************************************
 * Set transaction variables.
 ******************************************************************************/
void setTransactionVars(const char *address, const char *passphrase,
                        uint64_t fee) {
  strcpy(_txAddress, address);
  strcpy(_passphrase, passphrase);
  _fee = fee;
}

/********************************************************************************
 * This routine checks to see if Ark node is synchronized to the chain.
 *
 * This is a maybe a good way to see if node communication is working correctly;
 * it might be a good routine to run periodically
 *
 * json-formatted object:
 * {
 *   "data":{
 *     "synced":true,
 *     "now":4047140,
 *     "blocksCount":-4047140,
 *     "timestamp":82303508
 *   }
 * }
 *
 * @returns true if node is synced
 ********************************************************************************/
bool checkArkNodeStatus(Ark::Client::Connection<Ark::Client::Api> &connection) {
  // get status of Ark node
  const auto nodeStatus = connection.api.node.status();

  Serial.printf("\nNode Status: %s", nodeStatus.c_str());

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(4) + 50;
  DynamicJsonDocument doc(capacity);

  // "{\"data\":{\"synced\":true,\"now\":4047140,\"blocksCount\":-4047140,\"timestamp\":82303508}}";
  deserializeJson(doc, nodeStatus.c_str());
  JsonObject data = doc["data"];
  bool data_synced = data["synced"];

  return data_synced;
}

/********************************************************************************
 * This routine retrieves the current nonce and the balance for the wallet
 *
 * This is equivalent to calling
 * http://37.34.60.90:4040/api/v2/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1
 *
 * json-formatted object:
 * {
 *   "data":{
 *     "address":"TKneFA9Rm6GrX9zVXhn6iGprnW2fEauouE",
 *     "publicKey":"039ae554142f4df0a22c5c25b182896e9b3a1c785c6a0b8d1581cade5936608452",
 *     "nonce":"2",
 *     "balance":"2099999480773504",
 *     "isDelegate":false,
 *     "isResigned":false
 *   }
 * }
 ******************************************************************************/
bool getWallet(Ark::Client::Connection<Ark::Client::Api> &connection) {
  const auto walletGetResponse = connection.api.wallets.get(_txAddress);

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(6) + 200;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, walletGetResponse.c_str());
  JsonObject data = doc["data"];

  if (data["balance"]) {
    strcpy(bridgechainWallet.walletBalance, data["balance"]);
    bridgechainWallet.walletBalance_Uint64 =
        strtoull(data["balance"], NULL, 10);
    strcpy(bridgechainWallet.walletNonce, data["nonce"]);
    bridgechainWallet.walletNonce_Uint64 = strtoull(data["nonce"], NULL, 10);

    Serial.printf("\n\nGet Wallet: %s", walletGetResponse.c_str());
    Serial.printf("\nNonce: %s", bridgechainWallet.walletNonce);
    Serial.printf("\nBalance: %s\n", bridgechainWallet.walletBalance);
    return true;
  } else {
    Serial.printf("\n\nWallet Ballance is Empty.\n\n");
    return false;
  }
}

/********************************************************************************
 * This routine polls the ARK Node API for the RentalStart custom transaction.
 * It polls once every 8 seconds When polling the API we set the limit.
 ******************************************************************************/
int search_RentalStartTx(
    Ark::Client::Connection<Ark::Client::Api> &connection) {
  if (millis() - previousUpdateTime_RentalStartSearch >
      UpdateInterval_RentalStartSearch) {  // poll Ark node every 8 seconds for
                                           // a new transaction
    previousUpdateTime_RentalStartSearch += UpdateInterval_RentalStartSearch;

    //  check to see if new new transaction has been received in wallet
    // lastRXpage is the page# of the last received transaction
    int searchRXpage = bridgechainWallet.lastRXpage + 1;
    const char *id;               // transaction ID
    const char *amount;           // transactions amount
    const char *senderAddress;    // transaction address of sender
    const char *senderPublicKey;  // transaction address of sender
    const char *vendorField;      // vendor field

    const char *asset_gps_latitude;
    const char *asset_gps_longitude;
    const char *asset_sessionId;
    const char *asset_rate;

    if (GetTransaction_RentalStart(
            connection, _txAddress, searchRXpage, id, amount, senderAddress,
            senderPublicKey, vendorField, asset_gps_latitude,
            asset_gps_longitude, asset_sessionId, asset_rate)) {
      // increment received counter if rental start was received.
      bridgechainWallet.lastRXpage++;
      // store the page in the Flash
      saveEEPROM(bridgechainWallet.lastRXpage);

      Serial.printf("\nReceived sessionId: %s", asset_sessionId);
      Serial.printf("\nQRcodeHash: %s", QRcodeHash);

      // check if sessionID of the new transaction matches the Hash embedded
      // in QRcode that was displayed
      if (strcmp(asset_sessionId, QRcodeHash) == 0) {
        strcpy(scooterRental.senderAddress, senderAddress);
        strcpy(scooterRental.payment, amount);
        scooterRental.payment_Uint64 = strtoull(amount, NULL, 10);

        return 1;
      } else {
        // we received a transaction that did not match. We should issue refund.
        Serial.print("\nsession id did not match hash embedded in QRcode");
        // TODO: issueRefund();
        return 0;
      }
    } else {
      // we did not receive a transaction
      return 0;
    }
  } else {
    // it was not time to poll Ark network for a new transaction
    return 0;
  }
}

/********************************************************************************
 * This routine retrieves 1 received transaction in wallet if available
 *
 * returns parameters in
 * id -> transaction ID
 *  amount -> amount of ARKToshi
 *  senderAddress -> transaction sender address
 *  vendorfield -> 255(or 256??? check this) Byte vendor field
 *
 * @returns '0' if no transaction exist
 ******************************************************************************/
int GetReceivedTransaction(
    Ark::Client::Connection<Ark::Client::Api> &connection,
    const char *const address, int page, const char *&id, const char *&amount,
    const char *&senderAddress, const char *&senderPublicKey,
    const char *&vendorField) {
  // this is what we need to assemble:
  // https://radians.nl/api/v2/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp:asc
  // query = "?page=1&limit=1&orderBy=timestamp:asc"

  Serial.printf("\nPage: %d", page);

  char query[50];
  char page_char[8];
  strcpy(query, "?page=");
  itoa(page, page_char, 10);  // convert int to string
  strcat(query, page_char);
  strcat(query, "&limit=1&orderBy=timestamp:asc");

  // --------------------------------------------
  // perform the API
  // sort by oldest transactions first.
  // For simplicity set limit = 1 so we only get 1 transaction returned
  const auto walletGetResponse =
      connection.api.wallets.transactionsReceived(address, query);

  // add an extra 250 to account for platform variations
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) +
                          JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(9) +
                          JSON_OBJECT_SIZE(14) + 1240 + 250;

  DynamicJsonDocument doc(capacity);
  // "{\"meta\":{\"totalCountIsEstimate\":true,\"count\":1,\"pageCount\":2,\"totalCount\":2,\"next\":null,\"previous\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"self\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=2&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"first\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"last\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=2&limit=1&orderBy=timestamp%3Aasc&transform=true\"},\"data\":[{\"id\":\"a59b33f8e708d14fb726a7f5bd2c3bb35c35b6389553e6be6869a6699cdc69d5\",\"blockId\":\"14320034153575802056\",\"version\":2,\"type\":0,\"typeGroup\":1,\"amount\":\"100000000\",\"fee\":\"9806624\",\"sender\":\"TEf7p5jf1LReywuits5orBsmpkMe8fLTkk\",\"senderPublicKey\":\"02b7cca8003dbce7394f87d3a7127f6fab5a8ebace83e5633baaae38c58f3eee7a\",\"recipient\":\"TRXA2NUACckkYwWnS9JRkATQA453ukAcD1\",\"signature\":\"36772f190c7c11134f6c00db0cb03d3ac5ac7e972abc7ddef076afe4a4362e29afd6e55ef3a0f0fa76466d2f4bdd1afbf488e836fd2f83a195e58561ce7c7244\",\"confirmations\":16883,\"timestamp\":{\"epoch\":2191712,\"unix\":1574186052,\"human\":\"2019-11-19T17:54:12.856Z\"},\"nonce\":\"3\"}]}";
  deserializeJson(doc, walletGetResponse.c_str());

  JsonObject data_0 = doc["data"][0];
  const char *data_0_id = data_0["id"];
  const char *data_0_amount = data_0["amount"];
  const char *data_0_sender = data_0["sender"];
  const char *data_0_senderPublicKey = data_0["senderPublicKey"];
  const char *data_0_vendorField = data_0["vendorField"];

  // --------------------------------------------
  // Print the entire returned response string
  Serial.printf("\nGet Wallet Received Transaction: %s",
                walletGetResponse.c_str());

  // --------------------------------------------
  // The meta parameters that are returned are currently not reliable and are
  // "estimates". Apparently this is due to lower performance nodes For this
  // reason I will not use any of the meta parameters

  //--------------------------------------------
  //  the data_0_id parameter will be used to determine if a valid transaction
  //  was found.
  if (data_0_id == nullptr) {
    // no transaction found
    Serial.println("No Transaction. data_0_id is null");
    return 0;
  }

  Serial.println("transaction was received");
  id = data_0_id;
  amount = data_0_amount;
  senderAddress = data_0_sender;
  senderPublicKey = data_0_senderPublicKey;
  vendorField = data_0_vendorField;

  return 1;
}

/********************************************************************************
 * This routine will search through all the received transactions of ArkAddress
 * wallet starting from the oldest.
 *
 * "searching wallet + page#" will be displayed.
 * text will toggle between red/white every received transaction.
 * The page number of the last transaction in the search will be displayed.
 * This is the page to the most newest receive transaction on the chain.
 * The final page number is also equal to the total number of received
 * transactions in the wallet.
 *
 * The routine returns the page number of the most recent transaction.
 * Empty wallet will return '0' (NOT YET TESTED)
 ******************************************************************************/
int getMostRecentReceivedTransaction(
    Ark::Client::Connection<Ark::Client::Api> &connection, int page) {
  Serial.println("\n\nHere are all the transactions in a wallet");
  const char *id;               // transaction ID
  const char *amount;           // transactions amount
  const char *senderAddress;    // transaction address of sender
  const char *senderPublicKey;  // transaction address of sender
  const char *vendorField;      // vendor field

  while (GetReceivedTransaction(connection, _txAddress, page, id, amount,
                                senderAddress, senderPublicKey, vendorField)) {
    Serial.printf("\nPage: %s", page);
    Serial.printf("\nVendor Field: %s", vendorField);
    page++;
  };

  Serial.printf(
      "\nNo more Transactions.\nThe most recent transaction were on page#: %d",
      page - 1);

  return page - 1;
}

/********************************************************************************
 * This routine retrieves 1 RentalStart transaction if available in wallet
 * Returns '0' if no transaction exists or if other transaction type exists
 * Pass wallet address and page to function
 *
 * returns parameters:
 * - id -> transaction ID
 * - amount -> amount of Arktoshi
 * - senderAddress -> transaction sender address
 * - senderPublicKey -> transaction sender public key
 * - vendorField -> 255(or 256??? check this) Byte vendor field
 * - asset_gps_latitude
 * - asset_gps_longitude
 * - asset_sessionId
 * - asset_rate
 ******************************************************************************/
int GetTransaction_RentalStart(
    Ark::Client::Connection<Ark::Client::Api> &connection,
    const char *const address, int page, const char *&id, const char *&amount,
    const char *&senderAddress, const char *&senderPublicKey,
    const char *&vendorField, const char *&asset_gps_latitude,
    const char *&asset_gps_longitude, const char *&asset_sessionId,
    const char *&asset_rate) {
  // this is what we need to assemble:
  // https://radians.nl/api/v2/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=41&limit=1&orderBy=timestamp:asc
  // query = "?page=1&limit=1&orderBy=timestamp:asc"

  Serial.printf("\nPage: %d", page);

  char query[50];
  char page_char[8];
  strcpy(query, "?page=");
  itoa(page, page_char, 10);
  strcat(query, page_char);
  strcat(query, "&limit=1&orderBy=timestamp:asc");

  // --------------------------------------------
  // perform the API
  // sort by oldest transactions first. For simplicity set limit = 1 so we only
  // get 1 transaction returned
  const auto walletGetResponse =
      connection.api.wallets.transactionsReceived(address, query);

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) +
                          JSON_OBJECT_SIZE(3) + 2 * JSON_OBJECT_SIZE(4) +
                          JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(15) + 1580;
  DynamicJsonDocument doc(capacity);
  // "{\"meta\":{\"totalCountIsEstimate\":true,\"count\":1,\"pageCount\":72,\"totalCount\":72,\"next\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=42&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"previous\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=40&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"self\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=41&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"first\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"last\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=72&limit=1&orderBy=timestamp%3Aasc&transform=true\"},\"data\":[{\"id\":\"2238687a95688eb434953ac6548ade4648a3963a8158c036b65ee8e434e17230\",\"blockId\":\"10300106383332556177\",\"version\":2,\"type\":500,\"typeGroup\":4000,\"amount\":\"1\",\"fee\":\"10000000\",\"sender\":\"TLdYHTKRSD3rG66zsytqpAgJDX75qbcvgT\",\"senderPublicKey\":\"02cbe4667ab08693cbb3c248b96635f84b5412a99b49237f059a724f2cfe2b733f\",\"recipient\":\"TRXA2NUACckkYwWnS9JRkATQA453ukAcD1\",\"signature\":\"cc5b22000e267dad4ac52a319120fe3dd022ba6fcb102f635ffe66fc2ec1f6ae6b2491c53eb88352aadddab68d18dc6ce6ef4cba12bd84e53be1c28364350566\",\"asset\":{\"gps\":{\"timestamp\":1583125216,\"latitude\":\"1.111111\",\"longitude\":\"-180.222222\",\"human\":\"2020-03-02T05:00:16.000Z\"},\"sessionId\":\"2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824\",\"rate\":\"5\",\"gpsCount\":1},\"confirmations\":10618,\"timestamp\":{\"epoch\":11130880,\"unix\":1583125220,\"human\":\"2020-03-02T05:00:20.856Z\"},\"nonce\":\"40\"}]}";
  deserializeJson(doc, walletGetResponse.c_str());

  // --------------------------------------------
  //  Print the entire returned response string
  Serial.printf("\nGet Wallet Received Transaction: %s",
                walletGetResponse.c_str());

  // --------------------------------------------
  // The meta parameters that are returned are currently not reliable and are
  // "estimates". Apparently this is due to lower performance nodes For this
  // reason I will not use any of the meta parameters
  JsonObject data_0 = doc["data"][0];
  const char *data_0_id = data_0["id"];

  if (data_0_id == nullptr) {
    Serial.println("No Transaction. data_0_id is null");
    return 0;
  }

  Serial.println("Transaction was received");
  int data_0_type = data_0["type"];            // 500
  int data_0_typeGroup = data_0["typeGroup"];  // 4000

  // check for valid transaction type.
  if (!((data_0_type == 500) && (data_0_typeGroup == 4000))) {
    Serial.println("Received Transaction discarded, was not needed");
    bridgechainWallet.lastRXpage++;  // increment global receiver counter.
    saveEEPROM(bridgechainWallet.lastRXpage);
    return 0;
  }

  // Rental Start transaction was received
  JsonObject data_0_asset = data_0["asset"];
  JsonObject data_0_asset_gps = data_0_asset["gps"];

  asset_gps_latitude = data_0_asset_gps["latitude"];
  asset_gps_longitude = data_0_asset_gps["longitude"];
  asset_sessionId = data_0_asset["sessionId"];
  asset_rate = data_0_asset["rate"];
  id = data_0["id"];
  amount = data_0["amount"];
  senderAddress = data_0["sender"];
  senderPublicKey = data_0["senderPublicKey"];
  vendorField = data_0["vendorField"];

  return 1;
}

/********************************************************************************
 * Send a Rental Finish Custom BridgeChain transaction
 *
 * view rental finish transaction in explorer.
 * https://radians.nl/api/v2/transactions/61ebc45edcc87ca34a50b5e4590e5881dd4148c905bcf8208ad0afd2e7076348
 ******************************************************************************/
void SendTransaction_RentalFinish(
    Ark::Client::Connection<Ark::Client::Api> &connection) {
  // If the send fails then we need to unwind this increment.
  bridgechainWallet.walletNonce_Uint64++;

  uint64_t endlat = (uint64_t)(scooterRental.endLatitude * 1000000);
  uint64_t endlon = (uint64_t)(scooterRental.endLongitude * 1000000);

  uint64_t startlat = (uint64_t)(scooterRental.startLatitude * 1000000);
  uint64_t startlon = (uint64_t)(scooterRental.startLongitude * 1000000);

  Serial.println("Scooter Rental Finish GPS");
  Serial.print("scooterRental.endLatitude: ");
  Serial.println(scooterRental.endLatitude, 6);
  Serial.print("scooterRental.endLongitude: ");
  Serial.println(scooterRental.endLongitude, 6);

  Serial.printf("endlat: %" PRIu64 "\n", endlat);
  Serial.printf("endlon: %" PRIu64 "\n", endlon);

  // Use the Transaction Builder to make a transaction.
  auto bridgechainTransaction =
      builder::radians::ScooterRentalFinish(cfg)
          .recipientId(scooterRental.senderAddress)
          .timestamp(scooterRental.startTime, 0)
          .latitude(startlat, 0)
          .longitude(startlon, 0)
          .timestamp(scooterRental.endTime, 1)
          .latitude(endlat, 1)
          .longitude(endlon, 1)
          .sessionId(shaResult)
          // validation error when containsRefund = false.
          .containsRefund(false)
          .fee(10000000)
          .nonce(bridgechainWallet.walletNonce_Uint64)
          // bignumber error when amount = 0(even if state contains refund).
          .amount(1)
          .sign(_passphrase)
          .build();

  // Create and Print the Json representation of the Transaction.
  printf("\n\nBridgechain Transaction: %s\n\n",
         bridgechainTransaction.toJson().c_str());

  const auto bufferSize = 1500;
  char transactionsBuffer[bufferSize];
  snprintf(&transactionsBuffer[0], bufferSize, "{\"transactions\":[%s]}",
           bridgechainTransaction.toJson().c_str());

  std::string jsonTx = transactionsBuffer;
  std::string sendResponse = connection.api.transactions.send(jsonTx);

  Serial.println(sendResponse.c_str());
}

/*******************************************************************************
 * Send a BridgeChain transaction, tailored for a custom network.
 ******************************************************************************/
void sendBridgechainTransaction(
    Ark::Client::Connection<Ark::Client::Api> &connection) {
  // Use the Transaction Builder to make a transaction.
  bridgechainWallet.walletNonce_Uint64++;

  char tempVendorField[80];
  strcpy(tempVendorField, "Ride End: ");
  strcat(tempVendorField, QRcodeHash);

  auto bridgechainTransaction =
      builder::Transfer(cfg)
          .recipientId(scooterRental.senderAddress)  // genesis_2
          .vendorField(tempVendorField)
          .fee(_fee)
          .nonce(bridgechainWallet.walletNonce_Uint64)
          .amount(10000ULL)
          .expiration(0UL)
          .sign(_passphrase)
          .build();

  printf("\n\nBridgechain Transaction: %s\n\n",
         bridgechainTransaction.toJson().c_str());

  const auto bufferSize = 600;
  char transactionsBuffer[bufferSize];
  snprintf(&transactionsBuffer[0], bufferSize, "{\"transactions\":[%s]}",
           bridgechainTransaction.toJson().c_str());

  std::string jsonTx = transactionsBuffer;
  std::string sendResponse = connection.api.transactions.send(jsonTx);

  Serial.println(sendResponse.c_str());
}
