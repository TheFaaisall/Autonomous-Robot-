/*
 * arm.h
 * -----
 * Stepper motor control for the rotational arm mechanism.
 * Uses AccelStepper for smooth acceleration and deceleration,
 * which prevents the stepper from losing steps under load.
 *
 * The arm rotates between Zone A (home) and Zone D (target).
 * Step positions are defined in config.h as ARM_ZONE_A_STEPS
 * and ARM_ZONE_D_STEPS.
 */

#ifndef ARM_H
#define ARM_H

#include <Arduino.h>

/* Configure stepper driver pins and set initial position. */
void arm_init();

/*
 * Rotate arm from current position to Zone D.
 * Blocks until position is reached or timeout occurs.
 * Returns true on success, false on timeout.
 */
bool arm_move_to_zone_d();

/*
 * Rotate arm back to Zone A (home position).
 * Returns true on success, false on timeout.
 */
bool arm_move_to_zone_a();

/*
 * Decelerate and stop the stepper cleanly.
 * Does not cut power — stepper holds position.
 */
void arm_stop();

#endif /* ARM_H */
