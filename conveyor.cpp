/*
 * conveyor.cpp
 * ------------
 * DC motor control via L298N dual H-bridge.
 *
 * L298N wiring:
 *   IN1, IN2  — direction control (digital HIGH/LOW)
 *   ENA       — speed control (PWM via analogWrite)
 *
 * Direction logic:
 *   Forward (Zone A to D): IN1=HIGH, IN2=LOW
 *   Reverse (Zone D to A): IN1=LOW,  IN2=HIGH
 *   Stop:                  ENA=0  (or IN1=IN2=LOW)
 *
 * The motor runs until the appropriate limit switch triggers.
 * A timeout guard prevents infinite loops if a switch is wired wrong.
 */

#include "conveyor.h"
#include "config.h"
#include "sensors.h"

/* ----------------------------------------------------------------
   Internal helpers
   ---------------------------------------------------------------- */

static void _run(bool forward, uint8_t speed) {
    digitalWrite(PIN_CONV_IN1, forward ? HIGH : LOW);
    digitalWrite(PIN_CONV_IN2, forward ? LOW  : HIGH);
    analogWrite(PIN_CONV_ENA, speed);
}

/* ----------------------------------------------------------------
   Public functions
   ---------------------------------------------------------------- */

void conveyor_init() {
    pinMode(PIN_CONV_IN1, OUTPUT);
    pinMode(PIN_CONV_IN2, OUTPUT);
    pinMode(PIN_CONV_ENA, OUTPUT);
    conveyor_stop();

    #if TEST_MODE
    Serial.println(F("[Conveyor] Initialised"));
    #endif
}

bool conveyor_move_to_zone_d() {
    #if TEST_MODE
    Serial.println(F("[Conveyor] Moving to Zone D"));
    #endif

    _run(true, CONV_SPEED_FWD);

    unsigned long start = millis();
    while (!limit_zone_d_triggered()) {
        if (millis() - start > CONV_TIMEOUT_MS) {
            conveyor_stop();
            #if TEST_MODE
            Serial.println(F("[Conveyor] ERROR: Zone D timeout"));
            #endif
            return false;
        }
    }

    conveyor_stop();

    #if TEST_MODE
    Serial.println(F("[Conveyor] At Zone D"));
    #endif
    return true;
}

bool conveyor_move_to_zone_a() {
    #if TEST_MODE
    Serial.println(F("[Conveyor] Moving to Zone A"));
    #endif

    _run(false, CONV_SPEED_REV);

    unsigned long start = millis();
    while (!limit_zone_a_triggered()) {
        if (millis() - start > CONV_TIMEOUT_MS) {
            conveyor_stop();
            #if TEST_MODE
            Serial.println(F("[Conveyor] ERROR: Zone A timeout"));
            #endif
            return false;
        }
    }

    conveyor_stop();

    #if TEST_MODE
    Serial.println(F("[Conveyor] At Zone A"));
    #endif
    return true;
}

void conveyor_stop() {
    analogWrite(PIN_CONV_ENA, 0);
    digitalWrite(PIN_CONV_IN1, LOW);
    digitalWrite(PIN_CONV_IN2, LOW);
}
