/*
 * arm.cpp
 * -------
 * Stepper motor control for the novel rotational arm mechanism.
 *
 * Uses AccelStepper library (not the built-in Stepper.h) because:
 *   - AccelStepper handles acceleration and deceleration profiles.
 *     Without acceleration, a stepper under load skips steps at startup.
 *   - moveTo() + run() is non-blocking in structure — we call run()
 *     in a polling loop, which is safer than a fixed step delay.
 *   - It tracks absolute position so we can return home precisely.
 *
 * Driver wiring (A4988 or DRV8825):
 *   STEP  — one pulse per microstep
 *   DIR   — HIGH or LOW sets rotation direction
 *   ENABLE — pull LOW to power coils, HIGH to free shaft
 */

#include "arm.h"
#include "config.h"
#include <AccelStepper.h>

/*
 * AccelStepper::DRIVER mode = external step/dir driver.
 * Arguments: (mode, step_pin, dir_pin)
 * This is the correct mode for A4988/DRV8825 drivers.
 */
static AccelStepper _stepper(AccelStepper::DRIVER, PIN_STEP, PIN_DIR);

/* ----------------------------------------------------------------
   Initialisation
   ---------------------------------------------------------------- */
void arm_init() {
    pinMode(PIN_STEPPER_ENABLE, OUTPUT);
    digitalWrite(PIN_STEPPER_ENABLE, LOW);    /* Enable driver (active LOW) */

    _stepper.setMaxSpeed(ARM_MAX_SPEED);
    _stepper.setAcceleration(ARM_ACCELERATION);

    /* Define current physical position as step 0 (Zone A = home) */
    _stepper.setCurrentPosition(ARM_ZONE_A_STEPS);

    #if TEST_MODE
    Serial.println(F("[Arm] Initialised at Zone A (step 0)"));
    #endif
}

/* ----------------------------------------------------------------
   Internal: blocking move with timeout
   Calls _stepper.run() in a loop until target is reached.
   _stepper.run() must be called repeatedly — it moves one step
   per call when the timing is right. This is the AccelStepper pattern.
   ---------------------------------------------------------------- */
static bool _move_to(long target_steps, const __FlashStringHelper* label) {
    _stepper.moveTo(target_steps);
    unsigned long start = millis();

    while (_stepper.distanceToGo() != 0) {
        _stepper.run();   /* Execute one step if timing allows */

        if (millis() - start > ARM_TIMEOUT_MS) {
            arm_stop();
            #if TEST_MODE
            Serial.print(F("[Arm] ERROR: timeout moving to "));
            Serial.println(label);
            #endif
            return false;
        }
    }

    #if TEST_MODE
    Serial.print(F("[Arm] Reached "));
    Serial.println(label);
    #endif
    return true;
}

/* ----------------------------------------------------------------
   Public functions
   ---------------------------------------------------------------- */

bool arm_move_to_zone_d() {
    return _move_to(ARM_ZONE_D_STEPS, F("Zone D"));
}

bool arm_move_to_zone_a() {
    return _move_to(ARM_ZONE_A_STEPS, F("Zone A"));
}

void arm_stop() {
    _stepper.stop();
    /* Drain remaining deceleration steps so position is accurate */
    while (_stepper.distanceToGo() != 0) {
        _stepper.run();
    }
}
