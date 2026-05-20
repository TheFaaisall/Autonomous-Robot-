/*
 * gripper.cpp
 * -----------
 * Controls the servo motor that opens and closes the gripper.
 *
 * Why a static Servo instance here (not in main)?
 * The Servo object must persist for the entire program duration.
 * Keeping it in this file makes ownership explicit — gripper.cpp
 * owns the servo, nothing else touches it.
 */

#include "gripper.h"
#include "config.h"
#include <Servo.h>

static Servo  _servo;
static bool   _is_closed = false;

void gripper_init() {
    _servo.attach(PIN_GRIPPER_SERVO);
    gripper_open();   /* Start in safe open position */

    #if TEST_MODE
    Serial.println(F("[Gripper] Initialised — open"));
    #endif
}

void gripper_open() {
    _servo.write(GRIPPER_OPEN_DEG);
    _is_closed = false;
    delay(DROP_SETTLE_MS);   /* Give servo time to reach position */

    #if TEST_MODE
    Serial.println(F("[Gripper] Open"));
    #endif
}

void gripper_close() {
    _servo.write(GRIPPER_CLOSED_DEG);
    _is_closed = true;
    delay(GRAB_SETTLE_MS);   /* Give servo time to grip firmly */

    #if TEST_MODE
    Serial.println(F("[Gripper] Closed"));
    #endif
}

bool gripper_is_closed() {
    return _is_closed;
}
