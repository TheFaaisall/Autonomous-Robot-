/*
 * config.h
 * --------
 * Single source of truth for every constant in the firmware.
 * If you rewire a pin, change a timing, or retune a threshold,
 * this is the only file you touch.
 *
 * Required libraries (install via Arduino Library Manager):
 *   - AccelStepper by Mike McCauley
 *   - Servo (built into Arduino IDE)
 *
 * Board target: Arduino Mega 2560
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/* ================================================================
   Test mode
   Set to 1 to enable verbose Serial output and bypass start button.
   Set to 0 for competition — removes all Serial overhead.
   ================================================================ */
#define TEST_MODE 1

/* ================================================================
   Pin definitions
   ================================================================ */

/* Gripper — servo motor */
#define PIN_GRIPPER_SERVO       9

/* Conveyor — DC motor via L298N */
#define PIN_CONV_IN1            4
#define PIN_CONV_IN2            5
#define PIN_CONV_ENA            6     /* PWM speed control */

/* Arm — stepper motor via A4988 or DRV8825 driver */
#define PIN_STEP                10
#define PIN_DIR                 11
#define PIN_STEPPER_ENABLE      12    /* Active LOW — pull LOW to enable driver */

/* Ultrasonic sensor (HC-SR04) */
#define PIN_ULTRASONIC_TRIG     A0
#define PIN_ULTRASONIC_ECHO     A1

/* Limit switches — wired to interrupt-capable pins on Mega */
#define PIN_LIMIT_ZONE_A        2     /* INT0 */
#define PIN_LIMIT_ZONE_D        3     /* INT1 */

/* IR object detection sensor */
#define PIN_IR_DETECT           A2

/* Start button — press to begin mission */
#define PIN_START               7

/* Status LED */
#define PIN_LED_STATUS          13

/* ================================================================
   Gripper servo positions (degrees)
   Tune these to match your servo arm geometry.
   ================================================================ */
#define GRIPPER_OPEN_DEG        90
#define GRIPPER_CLOSED_DEG      10

/* ================================================================
   Conveyor — DC motor
   ================================================================ */
#define CONV_SPEED_FWD          200   /* PWM 0-255. Reduce if motor overshoots. */
#define CONV_SPEED_REV          200
#define CONV_TIMEOUT_MS         8000UL

/* ================================================================
   Arm — stepper motor
   These depend on your motor spec and microstepping setting on driver.
   1.8 deg/step motor at 1/8 microstepping = 1600 steps per revolution.
   ================================================================ */
#define STEPS_PER_REV           200   /* 1.8 deg/step full step */
#define MICROSTEP_FACTOR        8     /* Set by MS1/MS2/MS3 pins on driver */
#define STEPS_PER_FULL_ROT      (STEPS_PER_REV * MICROSTEP_FACTOR)

#define ARM_ZONE_A_STEPS        0L    /* Home position in steps */
#define ARM_ZONE_D_STEPS        400L  /* Steps to rotate from Zone A to Zone D — TUNE THIS */
#define ARM_MAX_SPEED           300.0f
#define ARM_ACCELERATION        150.0f
#define ARM_TIMEOUT_MS          10000UL

/* ================================================================
   Sensors
   ================================================================ */
#define IR_PAYLOAD_THRESHOLD    400   /* Analog value below = object detected. Tune for your IR. */
#define IR_CONFIRM_READS        3     /* Number of consecutive reads to confirm detection */
#define ULTRASONIC_TIMEOUT_US   30000UL
#define ULTRASONIC_NO_ECHO      -1.0f

/* ================================================================
   Mission timing (milliseconds)
   Competition limit is 120s. We abort at 110s to allow return.
   ================================================================ */
#define MISSION_TIMEOUT_MS      110000UL  /* Abort threshold */
#define GRAB_SETTLE_MS          600UL     /* Wait after closing gripper */
#define DROP_SETTLE_MS          500UL     /* Wait after opening gripper */
#define BUTTON_DEBOUNCE_MS      50UL

#endif /* CONFIG_H */
