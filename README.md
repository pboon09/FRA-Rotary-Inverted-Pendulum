# FRA421 Embedded Systems Final Project

## Overview

Advanced embedded system implementation with state estimation filters, multiple interfaces, and safety-critical design for Rotary Inverted Pendulum control.

## Features

### State Estimation
- ✅ **Kalman Filter** - Optimal velocity estimation from encoder positions
- ✅ **Low-Pass Filter** - Noise reduction
- ✅ **CMSIS DSP Library** - Optimized signal processing

### Multi-Interface System
- ✅ **Colors Shield LED (8×8)** - End-effector position visualization
- ✅ **LCD Display** - Real-time joint position and velocity display
- ✅ **SD Card Logging** - Position/velocity data logging
- ✅ **Bluetooth (HC-05)** - Wireless command interface

### Safety & Efficiency
- ✅ **Safety-critical programming** - Fault detection, graceful degradation
- ✅ **Low Power Modes** - Sleep modes during idle periods
- ✅ **Watchdog Timer** - System reliability

### Hardware Integration
- ✅ **Custom PCB** - All interfaces integrated on single board
- ✅ **Non-blocking architecture** - Concurrent interface operation

## System Architecture
```
STM32 NUCLEO-G474RE
    ├─ Encoders (AMT103-V) → Kalman Filter → LQR Controller → Motor
    ├─ LED Matrix → End-effector position (X-Y grid)
    ├─ LCD → Position and velocity display
    ├─ SD Card → CSV data logging
    └─ Bluetooth → Command reception (start/stop/config)
```