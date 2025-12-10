# Rotary Inverted Pendulum (RIP)

**Courses:** FRA333 Kinematics of Robotics System & FRA421 Embedded Systems  

## Project Overview

A complete implementation of a Rotary Inverted Pendulum system combining advanced control theory, kinematics analysis, and embedded systems design with multiple interface capabilities.

## Key Features

### Control & Kinematics (FRA333)
- ✅ Hybrid Energy-based Swing-up + LQR Stabilization
- ✅ Complete kinematics analysis (MDH, Jacobian, Dynamics)
- ✅ ROS2 integration with real-time visualization
- ✅ Theoretical vs Hardware validation

### Embedded Systems (FRA421)
- ✅ Kalman Filter & Low-Pass Filter for velocity estimation
- ✅ Multi-interface system: LED Display, LCD, SD Card logging, Bluetooth
- ✅ Safety-critical programming with Low Power modes
- ✅ Custom PCB integration

## Hardware

- **MCU:** STM32 NUCLEO-G474RE
- **Actuator:** Brushed DC Motor + Cytron MD20A Driver
- **Sensors:** 2× AMT103-V Encoders
- **Interfaces:** Colors Shield LED, LCD, SD Card, HC-05 Bluetooth
- **Dimensions:** 220×300mm base, 133mm arm, 135mm pendulum

## System Architecture
```
STM32 (Embedded Control + Filters)
    ├─> Motor Control (LQR/Energy-based)
    ├─> LED Display (End-effector position)
    ├─> LCD (Joint velocities)
    ├─> SD Card (Data logging)
    ├─> Bluetooth (Command interface)
    └─> micro-ROS → ROS2 → RViz (Visualization)
```

## Results

- **Control:** Stable swing-up and balance maintenance
- **Kinematics:** <0.01 rad/s velocity error (theory vs hardware)
- **Embedded:** Real-time multi-interface operation with filtered estimates

## Repository Structure

- `main` - Project overview and documentation
- `FRA333-Kinematics` - Kinematics analysis, ROS2 implementation, control theory
- `FRA421-Embedded` - Embedded firmware, filters, interface implementations