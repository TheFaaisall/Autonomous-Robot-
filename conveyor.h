/*
 * conveyor.h
 * ----------
 * DC motor control for the linear conveyor via L298N motor driver.
 * Moves the payload carriage between Zone A and Zone D.
 * Uses limit switches (via sensors.h) as end-stop detection.
 */

#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <Arduino.h>

/* Configure L298N control pins as outputs. Call once in setup(). */
void conveyor_init();

/*
 * Run motor forward until Zone D limit switch triggers.
 * Returns true on success, false on timeout (hardware fault).
 */
bool conveyor_move_to_zone_d();

/*
 * Run motor in reverse until Zone A limit switch triggers.
 * Returns true on success, false on timeout.
 */
bool conveyor_move_to_zone_a();

/* Cut motor drive immediately. */
void conveyor_stop();

#endif /* CONVEYOR_H */
