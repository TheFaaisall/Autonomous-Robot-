/*
 * gripper.h
 * ---------
 * Servo-driven gripper control.
 * Exposes open/close commands and a state query.
 */

#ifndef GRIPPER_H
#define GRIPPER_H

#include <Arduino.h>

/* Attach servo and move to open position. Call once in setup(). */
void gripper_init();

/* Move servo to GRIPPER_OPEN_DEG and wait DROP_SETTLE_MS. */
void gripper_open();

/* Move servo to GRIPPER_CLOSED_DEG and wait GRAB_SETTLE_MS. */
void gripper_close();

/* Returns true if gripper is in the closed position. */
bool gripper_is_closed();

#endif /* GRIPPER_H */
