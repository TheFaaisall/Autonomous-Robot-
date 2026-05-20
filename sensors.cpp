/*
 * sensors.cpp
 * -----------
 * All sensor hardware reads live here.
 * Nothing outside this file writes to sensor pins.
 */

#include "sensors.h"
#include "config.h"

/* ----------------------------------------------------------------
   Interrupt Service Routines for limit switches.

   Why interrupts instead of polling?
   Polling inside a blocking while-loop (conveyor_move_to_zone_d)
   would require checking the pin on every iteration.
   Interrupts fire instantly when the pin falls, regardless of what
   the main code is doing — guaranteed not to miss a trigger.

   volatile: tells the compiler this variable can change at any time
   from outside normal program flow (the ISR). Without volatile,
   the compiler might optimise away the check assuming it never changes.
   ---------------------------------------------------------------- */
static volatile bool _limit_a_flag = false;
static volatile bool _limit_d_flag = false;

static void isr_limit_a() { _limit_a_flag = true; }
static void isr_limit_d() { _limit_d_flag = true; }

/* ----------------------------------------------------------------
   Initialisation
   ---------------------------------------------------------------- */
void sensors_init() {
    /* Ultrasonic */
    pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
    pinMode(PIN_ULTRASONIC_ECHO, INPUT);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);

    /* IR sensor — analog input, no pinMode needed for analogRead */
    /* But set explicitly for clarity */
    pinMode(PIN_IR_DETECT, INPUT);

    /* Limit switches — INPUT_PULLUP means pin reads HIGH when switch is open.
       When switch closes, it connects pin to GND — pin reads LOW (FALLING edge).
       This means we don't need an external resistor. */
    pinMode(PIN_LIMIT_ZONE_A, INPUT_PULLUP);
    pinMode(PIN_LIMIT_ZONE_D, INPUT_PULLUP);

    /* Attach ISRs on FALLING edge (switch closes = pin goes HIGH to LOW) */
    attachInterrupt(digitalPinToInterrupt(PIN_LIMIT_ZONE_A), isr_limit_a, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_LIMIT_ZONE_D), isr_limit_d, FALLING);
}

/* ----------------------------------------------------------------
   IR payload detection
   Takes IR_CONFIRM_READS consecutive samples to confirm.
   Prevents single-read false positives from noise or vibration.
   ---------------------------------------------------------------- */
bool ir_payload_detected() {
    uint8_t hits = 0;
    for (uint8_t i = 0; i < IR_CONFIRM_READS; i++) {
        if (analogRead(PIN_IR_DETECT) < IR_PAYLOAD_THRESHOLD) {
            hits++;
        }
        delay(10);
    }
    return hits == IR_CONFIRM_READS;
}

/* ----------------------------------------------------------------
   Ultrasonic distance
   HC-SR04 protocol:
     1. Pull TRIG LOW for 2us (clean start)
     2. Pulse TRIG HIGH for 10us (fires ultrasonic burst)
     3. Measure how long ECHO stays HIGH
     4. Distance = duration * speed_of_sound / 2
        Speed of sound = 343 m/s = 0.0343 cm/us
   ---------------------------------------------------------------- */
float ultrasonic_distance_cm() {
    /* Ensure trigger is low before pulse */
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);

    /* 10us trigger pulse */
    digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_ULTRASONIC_TRIG, LOW);

    /* Measure echo duration. pulseIn returns 0 on timeout. */
    unsigned long duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, ULTRASONIC_TIMEOUT_US);

    if (duration == 0) {
        return ULTRASONIC_NO_ECHO;
    }

    return (float)duration * 0.0343f / 2.0f;
}

/* ----------------------------------------------------------------
   Limit switch readers
   Checks interrupt flag first (faster, set by ISR).
   Falls back to direct pin read (catches slow approaches that
   don't generate a clean edge).
   Clears flag after reading — one-shot per trigger.
   ---------------------------------------------------------------- */
bool limit_zone_a_triggered() {
    if (_limit_a_flag) {
        _limit_a_flag = false;
        return true;
    }
    return digitalRead(PIN_LIMIT_ZONE_A) == LOW;
}

bool limit_zone_d_triggered() {
    if (_limit_d_flag) {
        _limit_d_flag = false;
        return true;
    }
    return digitalRead(PIN_LIMIT_ZONE_D) == LOW;
}

void sensors_clear_flags() {
    _limit_a_flag = false;
    _limit_d_flag = false;
}
