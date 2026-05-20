/*
 * warman_robot.ino
 * ----------------
 * Warman Design and Build Competition — Monash University
 * Team: Faisal + 3 engineers
 * Result: 87% task completion, top 10 of 22 teams
 *
 * This file contains only setup() and loop().
 * It has no logic of its own — it wires all modules together.
 *
 * To understand the system, start here then follow the includes:
 *
 *   warman_robot.ino
 *   |-- config.h          Pin definitions and all tunable constants
 *   |-- sensors.h/.cpp    Ultrasonic, IR, limit switch reading
 *   |-- gripper.h/.cpp    Servo gripper open/close
 *   |-- conveyor.h/.cpp   DC motor carriage via L298N
 *   |-- arm.h/.cpp        Stepper rotational arm via A4988/DRV8825
 *   |-- state_machine.h/.cpp   Full mission task sequencer
 *
 * Required libraries (Arduino Library Manager):
 *   - AccelStepper by Mike McCauley
 *   - Servo (built-in)
 *
 * Upload target: Arduino Mega 2560
 * Baud rate:     115200
 */

#include "config.h"
#include "sensors.h"
#include "gripper.h"
#include "conveyor.h"
#include "arm.h"
#include "state_machine.h"

/* ----------------------------------------------------------------
   setup() — runs once on power-on or reset
   ---------------------------------------------------------------- */
void setup() {
    #if TEST_MODE
    Serial.begin(115200);
    Serial.println(F("==========================================="));
    Serial.println(F("  Warman Robot — Monash University"));
    Serial.println(F("  Subsystems: Gripper / Conveyor / Arm"));
    Serial.println(F("  Mission: Zone A to Zone D in 120s"));
    Serial.println(F("==========================================="));
    #endif

    /* Status indicators */
    pinMode(PIN_START, INPUT_PULLUP);
    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);

    /* Initialise all subsystems */
    sensors_init();
    gripper_init();
    conveyor_init();
    arm_init();
    state_machine_init();

    /* Single blink = ready and waiting for start button */
    digitalWrite(PIN_LED_STATUS, HIGH);
    delay(500);
    digitalWrite(PIN_LED_STATUS, LOW);

    #if TEST_MODE
    Serial.println(F("[Setup] All subsystems ready"));
    Serial.println(F("[Setup] Press start button to begin mission"));
    #endif
}

/* ----------------------------------------------------------------
   loop() — runs repeatedly after setup()
   All logic is inside state_machine_update().
   This keeps loop() clean and the state machine testable in isolation.
   ---------------------------------------------------------------- */
void loop() {
    state_machine_update();
}
