/*
 * sensors.h
 * ---------
 * Public interface for all sensor reading functions.
 * Every other module that needs sensor data includes this file.
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

/* Initialise all sensor pins and attach limit switch interrupts. */
void sensors_init();

/*
 * Returns true if the IR sensor detects a payload in the gripper.
 * Requires IR_CONFIRM_READS consecutive positive reads to return true.
 * Prevents false triggers from vibration or transient reflections.
 */
bool ir_payload_detected();

/*
 * Fires the HC-SR04 ultrasonic sensor and returns distance in cm.
 * Returns ULTRASONIC_NO_ECHO (-1.0) if no echo received within timeout.
 */
float ultrasonic_distance_cm();

/*
 * Limit switch state readers.
 * Returns true if the switch at Zone A or Zone D has been triggered.
 * Clears the interrupt flag after reading (one-shot behaviour).
 * Also reads the pin directly as a fallback.
 */
bool limit_zone_a_triggered();
bool limit_zone_d_triggered();

/* Reset interrupt flags manually (call after homing to clear stale flags). */
void sensors_clear_flags();

#endif /* SENSORS_H */
