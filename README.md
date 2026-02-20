# Embedded Systems Project Portfolio: Alarm Clock + Battleship (FreeRTOS, PSoC Control MCU)

This repository captures the development of **two embedded systems projects** completed in a PSoC Control MCU / FreeRTOS-based course environment:

- **HW01: Alarm Clock** (software-timed embedded application, no RTC)
- **HW05: Battleship Game** (task-based embedded game integration)

Together, these projects demonstrate progression in embedded C development, timing/state logic, peripheral integration, and real-time task coordination.

---

## Overview

This repository preserves the original course project structure (including provided platform support files, libraries, and starter scaffolding) while documenting my implementation work across two major projects:

### 1) HW01 – Alarm Clock
An embedded alarm clock application built **without a hardware RTC**, using software-managed timing behavior and control logic.

### 2) HW05 – Battleship Game
An embedded Battleship application implemented within a FreeRTOS-based environment, using task-level coordination, game-state logic, and peripheral communication/integration.

The Battleship project builds on concepts and development patterns introduced earlier in the alarm clock project.

---

## Why These Two Projects Belong Together

These were not isolated assignments—they reflect a **learning and development progression** in embedded systems:

- **HW01 (Alarm Clock)** helped develop:
  - software timing logic (without RTC hardware)
  - state transitions
  - periodic updates
  - embedded UI/control flow patterns

- **HW05 (Battleship)** applied and extended that experience into:
  - FreeRTOS task-based coordination
  - larger application integration
  - real-time behavior across multiple modules
  - embedded communication/peripheral interactions

This repo is kept in its original structure so the full project environment remains buildable and reproducible.

---

## My Contributions

My primary contributions across these projects include:

- Embedded application logic in **C**
- State-machine / control-flow implementation
- Software-managed timing behavior (HW01)
- FreeRTOS task-level logic and coordination (HW05)
- UART/peripheral integration
- Interrupt-driven interactions and debugging
- System integration within a provided embedded codebase/framework

---

## Project 1: HW01 Alarm Clock 

### Summary
The alarm clock project was implemented, which required software-based timing and periodic behavior management.

### Why It Matters
This project was important because it developed core embedded skills that were later useful in Battleship:
- timing and scheduling patterns
- state management
- UI/control sequencing
- modular embedded code organization

### Key Skills Demonstrated
- Embedded C programming
- Software timing logic (no RTC)
- State transitions and periodic updates
- Integration in a provided codebase

---

## Project 2: HW05 Battleship Game (FreeRTOS)

### Summary
The Battleship project is the primary game-based embedded application in this repository, implemented using a **FreeRTOS task-based architecture**.

### Key Skills Demonstrated
- FreeRTOS task scheduling and coordination
- Embedded game-state logic
- Real-time interaction handling
- Peripheral/UART integration
- System debugging and integration within an existing platform framework

---

## Tech Stack

- **Language:** C
- **RTOS:** FreeRTOS (HW05)
- **Platform:** PSoC Control MCU
- **Interfaces/Concepts:** UART, GPIO, interrupts, task scheduling, state machines
- **Build Flow:** Makefile-based build environment
- **Tooling/Environment:** ModusToolbox / embedded toolchain ecosystem (provided scaffold)


