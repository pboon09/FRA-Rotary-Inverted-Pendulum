import numpy as np
from roboticstoolbox import DHRobot, RevoluteMDH
from spatialmath import SE3


L0 = 64e-3  
L1 = 133e-3 
L2 = 135e-3

# 2. Define Modified DH Parameters
# Modified DH: [alpha(i-1), a(i-1), d(i), theta(i)]
link01 = RevoluteMDH(a=0, alpha=0, d=L0, offset=0)
link12 = RevoluteMDH(a=L1, alpha=np.pi/2, d=0, offset=np.pi/2)
link23 = RevoluteMDH(a=0, alpha=np.pi/2, d=0, offset=0)
# link3e = RevoluteMDH(a=L2, alpha=0, d=0, offset=0)

robot = DHRobot([link01, link12, link23], name='RIP')
robot.tool = SE3.Tx(L2)

print("Robot Model:")
print(robot)

# 4. Calculate Jacobian at Any Configuration
# Joint configuration [q1, q2] in radians
q1 = 0.0
q2 = 0.0
q = np.array([q1, 0, q2]) 

print("\n" + "=" * 60)
print(f"Configuration: θ={np.rad2deg(q[0]):.1f}°, β={np.rad2deg(q[1]):.1f}°")
print("=" * 60)


# Toolbox calculates Jacobian automatically!
J = robot.jacob0(q)  # Jacobian in base frame
print(f"\nFull Jacobian shape: {J.shape}")  # [6, 4]

# Extract only columns for actual joints (q2=theta, q3=beta)
J_actual = J[:, [0, 2]]  # Columns 1 and 2 (indices 1, 2)
print(f"Actual joints Jacobian shape: {J_actual.shape}")  # [6, 2] ✅
print("\nLinear velocity part (q1, q2):")
print(J_actual[:3, :])
print("\nAngular velocity part (q1, q2):")
print(J_actual[3:, :])