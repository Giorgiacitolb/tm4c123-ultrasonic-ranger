# TM4C123 Ultrasonic Ranger Finder

Embedded firmware project using the TM4C123 (ARM Cortex-M4) microcontroller to measure distance with an HC-SR04 ultrasonic sensor and report results over UART.

## Overview

This project implements an interrupt-driven ultrasonic distance measurement system. The firmware generates trigger pulses, measures echo pulse width using hardware timers, and converts time-of-flight to distance in centimeters.

## Features

- PLL configuration for 16 MHz system clock
- 16-bit General Purpose Timer (GPTM) configuration
- Input capture for echo pulse-width measurement
- Integer-based distance calculation (optimized for microcontroller)
- UART0 serial output (57600 baud)
- SysTick-based LED status indication
- Modular driver structure (Blink, UART, PLL, Ultrasonic, SysTick)

## Hardware

- TM4C123 LaunchPad
- HC-SR04 Ultrasonic Sensor
- Voltage divider for echo pin (5V → 3.3V)

## My Contribution

Worked as part of a team project. My contributions included:
- Timer configuration and pulse-width measurement logic
- UART reporting implementation
- Debugging hardware–software integration using oscilloscopes and logic analyzers

## How It Works (High-Level)

1. Trigger pulse sent to ultrasonic sensor  
2. Echo signal captured using timer input capture  
3. Pulse duration converted to distance  
4. Distance displayed via UART and LED status

---

This project demonstrates embedded systems development including timer configuration, interrupt handling, and hardware integration.