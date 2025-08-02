# TinyEmbed: Virtual Embedded Board Simulator

TinyEmbed is a lightweight C++ simulation framework for embedded systems development and testing. It models a simple virtual board that supports:

- GPIO (General Purpose Input/Output) control
- UART (Universal Asynchronous Receiver Transmitter) communication via named pipes
- Software timer ticks (simulated clock)
- Multithreaded simulation and timing
- Internal logs for tracing system activity

---

## Features

- **GPIO**: Read/write/toggle binary states for a specified number of pins.
- **UART**: Simulate byte-wise communication between two boards using named pipes.
- **Timer**: Internal ticking clock using `std::thread`, enables accurate simulation steps.
- **Thread-safe UART buffering** with mutex protection.
- **Minimal dependency**: Standard C++17 and POSIX API.

---


## Build Instructions

### Requirements

- C++17 compiler
- POSIX-compatible system (Linux/macOS)

### Compile Manually

```bash
g++ example.cpp Board.cpp -Iinclude -o tinyembed_test -pthread
