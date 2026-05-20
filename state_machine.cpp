/*
 * state_machine.cpp
 * -----------------
 * Warman competition task sequencer.
 *
 * This is the brain of the robot. It coordinates all three subsystems
 * (gripper, conveyor, arm) in the correct order to complete the mission:
 *
 *   1. Home all axes to Zone A
 *   2. Wait for payload to be placed in gripper
 *   3. Grab payload
 *   4. Transport from Zone A to Zone D
 *   5. Drop payload
 *   6. Return to Zone A
 *   7. Signal complete
 *
 * All within 120 seconds (we abort at 110s to allow safe return).
 */

#include "state_machine.h"
#include "config.h"
#include "sensors.h"
#include "gripper.h"
#include "conveyor.h"
#include "arm.h"

/* ----------------------------------------------------------------
   Internal state
   ---------------------------------------------------------------- */
static RobotState      _state          = STATE_IDLE;
static unsigned long   _mission_start  = 0;

/* ----------------------------------------------------------------
   Helpers
   ---------------------------------------------------------------- */

static bool mission_timed_out() {
    if (_mission_start == 0) return false;
    return (millis() - _mission_start) >= MISSION_TIMEOUT_MS;
}

static void log(const __FlashStringHelper* msg) {
    #if TEST_MODE
    unsigned long elapsed = (_mission_start > 0) ? (millis() - _mission_start) / 1000 : 0;
    Serial.print(F("["));
    Serial.print(elapsed);
    Serial.print(F("s] "));
    Serial.println(msg);
    #endif
}

static void emergency_return() {
    log(F("TIMEOUT — aborting, returning to Zone A"));
    arm_stop();
    conveyor_stop();
    gripper_open();
    _state = STATE_RETURN_TO_A;
}

/* ----------------------------------------------------------------
   State name lookup — stored in flash to save RAM
   ---------------------------------------------------------------- */
const __FlashStringHelper* state_name(RobotState s) {
    switch (s) {
        case STATE_IDLE:             return F("IDLE");
        case STATE_HOMING:           return F("HOMING");
        case STATE_WAIT_FOR_PAYLOAD: return F("WAIT_PAYLOAD");
        case STATE_GRAB:             return F("GRAB");
        case STATE_TRANSPORT_TO_D:   return F("TRANSPORT_D");
        case STATE_DROP:             return F("DROP");
        case STATE_RETURN_TO_A:      return F("RETURN_A");
        case STATE_COMPLETE:         return F("COMPLETE");
        case STATE_FAULT:            return F("FAULT");
        default:                     return F("UNKNOWN");
    }
}

/* ----------------------------------------------------------------
   Public interface
   ---------------------------------------------------------------- */

void state_machine_init() {
    _state         = STATE_IDLE;
    _mission_start = 0;
    log(F("State machine ready — waiting for start button"));
}

RobotState state_machine_get_state() {
    return _state;
}

void state_machine_update() {

    /* Global mission timeout — applies to all active states */
    bool active_state = (_state != STATE_IDLE &&
                         _state != STATE_COMPLETE &&
                         _state != STATE_FAULT);
    if (active_state && mission_timed_out()) {
        emergency_return();
        return;
    }

    switch (_state) {

        /* --------------------------------------------------------
           IDLE: wait for start button
           Debounced LOW read on PIN_START begins the mission.
           -------------------------------------------------------- */
        case STATE_IDLE:
            if (digitalRead(PIN_START) == LOW) {
                delay(BUTTON_DEBOUNCE_MS);
                if (digitalRead(PIN_START) == LOW) {
                    _mission_start = millis();
                    log(F("Start button pressed — mission clock started"));
                    _state = STATE_HOMING;
                }
            }
            break;

        /* --------------------------------------------------------
           HOMING: drive conveyor and arm to Zone A
           Uses limit switches to confirm position.
           Opens gripper as a safety measure.
           -------------------------------------------------------- */
        case STATE_HOMING:
            log(F("Homing all axes to Zone A"));
            gripper_open();
            sensors_clear_flags();

            if (!conveyor_move_to_zone_a()) {
                log(F("FAULT: conveyor failed to home"));
                _state = STATE_FAULT;
                break;
            }
            if (!arm_move_to_zone_a()) {
                log(F("FAULT: arm failed to home"));
                _state = STATE_FAULT;
                break;
            }

            log(F("Homing complete"));
            _state = STATE_WAIT_FOR_PAYLOAD;
            break;

        /* --------------------------------------------------------
           WAIT_FOR_PAYLOAD: poll IR sensor
           Requires IR_CONFIRM_READS consecutive positive reads.
           Stays here until payload is placed in gripper zone.
           -------------------------------------------------------- */
        case STATE_WAIT_FOR_PAYLOAD:
            if (ir_payload_detected()) {
                log(F("Payload confirmed in gripper zone"));
                _state = STATE_GRAB;
            }
            break;

        /* --------------------------------------------------------
           GRAB: close gripper and verify hold
           If payload is not detected after closing, open and retry.
           -------------------------------------------------------- */
        case STATE_GRAB:
            log(F("Closing gripper"));
            gripper_close();

            if (!ir_payload_detected()) {
                log(F("WARN: payload not detected after grab — retrying"));
                gripper_open();
                _state = STATE_WAIT_FOR_PAYLOAD;
                break;
            }

            log(F("Payload secured"));
            _state = STATE_TRANSPORT_TO_D;
            break;

        /* --------------------------------------------------------
           TRANSPORT_TO_D: move payload from Zone A to Zone D
           Conveyor drives carriage to Zone D limit switch.
           Arm rotates to Zone D angular position.
           Order: conveyor first, then arm rotation (tune if needed).
           -------------------------------------------------------- */
        case STATE_TRANSPORT_TO_D:
            log(F("Transporting payload to Zone D"));

            if (!conveyor_move_to_zone_d()) {
                log(F("FAULT: conveyor failed to reach Zone D"));
                _state = STATE_FAULT;
                break;
            }
            if (!arm_move_to_zone_d()) {
                log(F("FAULT: arm failed to reach Zone D position"));
                _state = STATE_FAULT;
                break;
            }

            log(F("Arrived at Zone D"));
            _state = STATE_DROP;
            break;

        /* --------------------------------------------------------
           DROP: open gripper, release payload, confirm release
           If IR still detects payload after opening, retry once.
           -------------------------------------------------------- */
        case STATE_DROP:
            log(F("Releasing payload at Zone D"));
            gripper_open();

            if (ir_payload_detected()) {
                log(F("WARN: payload still detected — retry release"));
                delay(300);
                gripper_open();
            }

            log(F("Payload released"));
            _state = STATE_RETURN_TO_A;
            break;

        /* --------------------------------------------------------
           RETURN_TO_A: bring all axes back to Zone A
           Reverse order of transport — arm first, then conveyor.
           -------------------------------------------------------- */
        case STATE_RETURN_TO_A:
            log(F("Returning to Zone A"));

            if (!arm_move_to_zone_a()) {
                log(F("FAULT: arm failed to return to Zone A"));
                _state = STATE_FAULT;
                break;
            }
            if (!conveyor_move_to_zone_a()) {
                log(F("FAULT: conveyor failed to return to Zone A"));
                _state = STATE_FAULT;
                break;
            }

            log(F("Returned to Zone A"));
            _state = STATE_COMPLETE;
            break;

        /* --------------------------------------------------------
           COMPLETE: mission finished
           Log elapsed time. Flash LED. Stay here permanently.
           -------------------------------------------------------- */
        case STATE_COMPLETE: {
            unsigned long elapsed_s = (millis() - _mission_start) / 1000UL;
            #if TEST_MODE
            Serial.print(F("[COMPLETE] Mission finished in "));
            Serial.print(elapsed_s);
            Serial.println(F(" seconds"));
            #endif

            /* Blink 5 times to signal success, then hold steady */
            for (uint8_t i = 0; i < 5; i++) {
                digitalWrite(PIN_LED_STATUS, HIGH);
                delay(300);
                digitalWrite(PIN_LED_STATUS, LOW);
                delay(300);
            }
            digitalWrite(PIN_LED_STATUS, HIGH);

            /* Infinite hold — reset Arduino to run again */
            while (true) { /* hold */ }
            break;
        }

        /* --------------------------------------------------------
           FAULT: unrecoverable hardware failure
           Rapid LED blink. All motors already stopped by caller.
           Requires manual reset.
           -------------------------------------------------------- */
        case STATE_FAULT:
            digitalWrite(PIN_LED_STATUS, (millis() / 150) % 2 ? HIGH : LOW);
            break;
    }
}
