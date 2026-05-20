/*
 * state_machine.h
 * ---------------
 * Task sequencer for the full Warman competition mission.
 *
 * States and their transitions:
 *
 *   IDLE
 *     |-- start button pressed
 *   HOMING
 *     |-- conveyor and arm at Zone A
 *   WAIT_FOR_PAYLOAD
 *     |-- IR sensor confirms payload present
 *   GRAB
 *     |-- gripper closed, payload confirmed held
 *   TRANSPORT_TO_D
 *     |-- conveyor at Zone D limit switch, arm at Zone D steps
 *   DROP
 *     |-- gripper open, payload confirmed released
 *   RETURN_TO_A
 *     |-- arm and conveyor back at Zone A
 *   COMPLETE
 *     |-- stays here, blinks LED, prints elapsed time
 *
 *   Any state can transition to FAULT on hardware failure.
 *   Any state (except IDLE/COMPLETE/FAULT) checks mission timeout.
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

typedef enum {
    STATE_IDLE,
    STATE_HOMING,
    STATE_WAIT_FOR_PAYLOAD,
    STATE_GRAB,
    STATE_TRANSPORT_TO_D,
    STATE_DROP,
    STATE_RETURN_TO_A,
    STATE_COMPLETE,
    STATE_FAULT
} RobotState;

/* Initialise state machine. Call once in setup(). */
void state_machine_init();

/*
 * Execute one iteration of the state machine.
 * Call this from loop() repeatedly.
 * Each state is either blocking (runs to completion) or
 * polling (returns and is called again next loop).
 */
void state_machine_update();

/* Returns the current state (for external status display if needed). */
RobotState state_machine_get_state();

/* Returns a human-readable state name stored in flash (F() macro). */
const __FlashStringHelper* state_name(RobotState state);

#endif /* STATE_MACHINE_H */
