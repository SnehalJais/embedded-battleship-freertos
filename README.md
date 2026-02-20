# Embedded Battleship (FreeRTOS, PSoC Control MCU)

Embedded Battleship game implemented in C using FreeRTOS with concurrent tasks, interrupt-driven peripherals, and UART communication.

## Overview
This project was developed in an embedded systems course using a provided PSoC Control MCU project scaffold (including board support files, build structure, and starter application code). I implemented and extended the project-specific game functionality, task logic, and embedded integration for the Battleship application.

## What I Implemented
- FreeRTOS task-based game behavior and coordination
- Embedded game logic/state handling
- UART communication/protocol integration
- Interrupt-driven peripheral interactions
- Project integration/debugging within the provided platform framework

## Tech Stack
- C
- FreeRTOS
- PSoC Control MCU platform
- UART / GPIO / interrupts
- Makefile-based build flow

## Repository Structure
- `src/tasks/` – primary project task logic and application behavior
- `src/drivers/` – hardware driver code used by the project (includes provided + integrated components)
- `src/examples/`, `src/ice/`, `src/homework/` – course-related code/resources used during development
- `bsps/`, `deps/`, `libs/` – provided platform/vendor support files required for build and execution

## Notes
This repository preserves the original course/project structure so the full project remains buildable. It includes provided scaffold/platform files in addition to my implemented project logic.
