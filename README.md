# FRA333 Kinematics of Robotics System

## Overview

Implementation of kinematics analysis, dynamics modeling, and hybrid control for a Rotary Inverted Pendulum with ROS2 integration.

## Control Strategy

### Energy-Based Swing-Up
- Control law: `U = kE·ΔE·α̇·cos(α)`
- Brings pendulum from downward to upright position

### LQR Stabilization
- Linearized state-space model around upright equilibrium
- Full state feedback with discrete-time Riccati equation

## Kinematics

### MDH Parameters
- 3 revolute joints modeled with Modified DH convention
- Validated using geometric approach

### Forward Kinematics & Jacobian
- End-effector position: `P = [Px, Py, Pz]`
- Jacobian (6×2) for velocity mapping
- Inverse Jacobian for theoretical velocity estimation

### Dynamics (Lagrangian Method)
```
τ = M(q)q̈ + C(q,q̇)q̇ + G(q)
```

## ROS2 Implementation

### Nodes
- `inv_pendulum` (micro-ROS) - Hardware interface & control
- `joint_state_republisher` - URDF formatting
- `theory_joint_state_publisher` - FK/Jacobian computation
- `velocity_hud` - Visualization overlay
- `robot_state_publisher` - URDF & TF
- **RViz2** - 3D visualization

## Results

✅ Position: Perfect theory-hardware agreement  
✅ Velocity: <0.01 rad/s error  
✅ Real-time visualization with frame overlay  
✅ Stable balance control

## Documentation

See [full report PDF](G5_6619_6631_6637_6642.pdf) for detailed derivations and results.