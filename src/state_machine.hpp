/*******************************************************************************
 * This contains Finite State Machine logic for controlling a rental session
 ******************************************************************************/

#ifndef ARK_SCOOTERS_STATE_STATE_MACHINE_HPP
#define ARK_SCOOTERS_STATE_STATE_MACHINE_HPP

#include <cstdint>

#include <arkClient.h>

/*******************************************************************************
 * State Machine
 ******************************************************************************/
typedef enum state_enum_t {
  STATE_0,
  STATE_1,
  STATE_2,
  STATE_3,
  STATE_4,
  STATE_5,
  STATE_6
} State_enum;

extern State_enum state;

/*******************************************************************************
 * Set state variables.
 ******************************************************************************/
void setStateMachineVars(const char *address, const char *rateStr,
                         uint64_t rate64);

/*******************************************************************************
 * Mealy Finite State Machine
 * The state machine logic is executed once each cycle of the "main" loop.
 ******************************************************************************/
void StateMachine(Ark::Client::Connection<Ark::Client::Api> &connection);

/*******************************************************************************
 * unlock the scooter
 ******************************************************************************/
void unlockScooter();

#endif  // ARK_SCOOTERS_STATE_STATE_MACHINE_HPP
