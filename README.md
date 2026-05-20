# Warman Design and Build Competition — Autonomous Robot Firmware

Monash University | Mechanical and Mechatronics Engineering

---

![Robot on competition table](image_6_placeholder.jpg)

---

## Overview

This repository contains the Arduino firmware for an autonomous robot built for the Warman Design and Build Competition. The robot's task was to pick up a payload from Zone A, transport it to Zone D, release it, and return to the starting position within 120 seconds.

The robot integrates three mechanical subsystems (gripper, conveyor, and launcher arm) controlled by a sensor-driven embedded firmware written in C on Arduino. A finite state machine coordinates all actuators and sensors through the full mission sequence autonomously.

Note: the code in this repository was rewritten by me after the competition. The firmware used on competition day was developed collaboratively with the team under time pressure. This version is a clean, structured rewrite that reflects the same logic and hardware behaviour with improved modularity, documentation, and fault handling.

---

## Competition Task

![Competition zone layout and robot positioning](imagetop.png)

The robot operated on a fixed platform. The task required:

1. Gripping a payload placed in Zone A
2. Transporting the payload from Zone A to Zone D via a linear conveyor and rotational arm
3. Releasing the payload at Zone D
4. Returning all axes to the starting position

All steps had to be completed autonomously within 120 seconds from the start signal.

---

## Hardware

### Design

![SolidWorks CAD model of the robot](fullcad.png)

The robot was designed in SolidWorks before fabrication. The chassis uses an acrylic and aluminium frame. The key mechanical feature is a novel rotational mechanism that allows the arm to swing between Zone A and Zone D, reducing linear travel and improving cycle time.

### Physical Build

![Top-down view showing full assembly and gripper holding payload](image_1_placeholder.jpg)

![Top-down view showing gripper open position](image_2_placeholder.jpg)

![Top-down view showing internal electronics and wiring](image_3_placeholder.jpg)

![Alternate angle showing underside and motor mounting](image_4_placeholder.jpg)

### Components

- Arduino Mega 2560 microcontroller
- DC motor with L298N H-bridge driver (conveyor linear drive via wire rope and pulleys)
- Stepper motor with A4988 or DRV8825 driver (rotational arm mechanism)
- Servo motor (gripper open and close)
- HC-SR04 ultrasonic sensor (distance feedback)
- Limit switches at Zone A and Zone D (end-stop detection via hardware interrupts)
- IR sensor (payload presence detection in gripper)
- 3D-printed gripper arms and structural brackets
- Aluminium linear rail extrusion (conveyor carriage track)
- Steel wire rope and pulleys (conveyor drive transmission)

---

## Firmware Architecture

The firmware is split across 12 files. Each file has one responsibility. All files must be placed in the same Arduino sketch folder.

```
warman_robot/
    warman_robot.ino      Entry point. Contains setup() and loop() only. No logic.
    config.h              All pin definitions, timing constants, and tunable thresholds.
    sensors.h             Public interface for all sensor functions.
    sensors.cpp           Ultrasonic distance, IR payload detection, limit switch ISRs.
    gripper.h             Public interface for gripper control.
    gripper.cpp           Servo motor open and close with settle delays.
    conveyor.h            Public interface for DC motor conveyor.
    conveyor.cpp          L298N direction and PWM control, limit-switch-guided travel.
    arm.h                 Public interface for stepper arm.
    arm.cpp               AccelStepper-based rotational arm with acceleration profiles.
    state_machine.h       Robot state enum and state machine interface.
    state_machine.cpp     Full 9-state mission sequencer with timeout and fault handling.
```

No file exceeds 275 lines.

---

## Module Dependency Map

```
config.h              (no dependencies)
sensors.h / .cpp      --> config.h
gripper.h / .cpp      --> config.h
conveyor.h / .cpp     --> config.h, sensors.h
arm.h / .cpp          --> config.h
state_machine.h/.cpp  --> config.h, sensors.h, gripper.h, conveyor.h, arm.h
warman_robot.ino      --> all modules
```

Dependencies flow in one direction only. No circular includes.

---

## Mission State Machine

```
IDLE
  |-- start button pressed
HOMING
  |-- conveyor at Zone A limit switch, arm at step 0
WAIT_FOR_PAYLOAD
  |-- IR sensor confirms payload (3 consecutive reads)
GRAB
  |-- gripper closed, payload confirmed held
TRANSPORT_TO_D
  |-- conveyor at Zone D limit switch, arm at Zone D steps
DROP
  |-- gripper open, payload release confirmed
RETURN_TO_A
  |-- arm and conveyor back at Zone A
COMPLETE
  |-- LED blinks, elapsed time logged, system holds

Any state --> FAULT on hardware timeout
Any active state --> RETURN_TO_A if mission exceeds 110 seconds
```

The 110-second abort threshold (not 120) is intentional. It reserves 10 seconds for the return journey regardless of where in the sequence the robot is when the clock runs low.

---

## Configuration

All tunable parameters are in config.h. Change pin numbers there if your wiring differs. Key values to tune for your hardware:

```c
ARM_ZONE_D_STEPS     // steps from Zone A to Zone D on the rotational arm
CONV_SPEED_FWD       // PWM speed for conveyor (0-255)
IR_PAYLOAD_THRESHOLD // analog threshold for IR payload detection
GRIPPER_CLOSED_DEG   // servo angle for closed gripper position
```

Set TEST_MODE to 1 in config.h to enable Serial debug output at 115200 baud. Set to 0 for competition to remove all Serial overhead.

---

## Setup

1. Install required libraries via Arduino Library Manager:
   - AccelStepper by Mike McCauley
   - Servo (built-in to Arduino IDE)

2. Open warman_robot.ino in Arduino IDE. All other files in the folder are compiled automatically.

3. Select board: Arduino Mega 2560

4. Set your pin numbers in config.h to match your wiring.

5. Set TEST_MODE 1, upload, and open Serial Monitor at 115200 baud to verify each subsystem before competition.

6. Set TEST_MODE 0 before final upload for competition day.

---

## Skills Demonstrated

- Embedded firmware development in C on Arduino Mega
- Finite state machine design for autonomous sequencing
- Multi-axis actuator coordination across DC motor, stepper motor, and servo
- Hardware interrupt-driven sensor integration
- Modular firmware architecture across 12 files with clean dependency structure
- Real-time fault detection and safe abort handling within a 120-second mission window
- SolidWorks mechanical design and physical prototype integration
- Electronics wiring and subsystem integration across three mechanical assemblies

---

## Team

This was a team project completed by a 4-engineer group at Monash University as part of the Warman Design and Build Competition. My role covered team leadership, electronics wiring, subsystem integration, and firmware development. The firmware in this repository is my own rewrite produced after the competition.
