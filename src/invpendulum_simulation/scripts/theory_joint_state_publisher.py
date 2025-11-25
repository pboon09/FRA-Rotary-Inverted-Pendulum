#!/usr/bin/env python3

import math
import numpy as np
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from std_msgs.msg import Header, Float32MultiArray
from geometry_msgs.msg import TransformStamped
from tf2_ros import TransformBroadcaster
from roboticstoolbox import DHRobot, RevoluteMDH
from spatialmath import SE3
from invpendulum_simulation.joint_name_config import NAME_MAP, ORDER_IN
from scipy.spatial.transform import Rotation as R


class TheoryJointPublisher(Node):
    def __init__(self):
        super().__init__('theory_joint_publisher')

        # Publishers
        self.js_pub = self.create_publisher(JointState, 'joint_states_theory', 10)
        self.ee_pub = self.create_publisher(Float32MultiArray, 'end_effector_velocity', 10)
        self.tf_pub = TransformBroadcaster(self)
        
        # Subscription
        self.create_subscription(JointState, "joint_states", self.js_callback, 10)
        
        # Physical parameters FROM URDF (m)
        self.arm_joint_x = 0.0002
        self.arm_joint_y = 0.0
        self.arm_joint_z = 0.212
        
        self.pend_joint_x_local = 0.1215
        self.pend_joint_y_local = 0.0
        self.pend_joint_z_local = 0.0255
        
        # DH parameters
        self.L0 = 237.5e-3
        self.L1 = 133.2e-3
        self.L2 = 135e-3
        
        # Current joint POSITIONS (from encoder)
        self.q1 = 0.0
        self.q2 = 0.0
        
        # MEASURED joint velocities (from encoder)
        self.dq1_measured = 0.0
        self.dq2_measured = 0.0
        
        # CALCULATED "theory" joint velocities (from FK differentiation)
        self.dq1_theory = 0.0
        self.dq2_theory = 0.0
        
        # Previous end-effector position and ROTATION MATRIX for velocity calculation
        self.ee_pos_prev = np.zeros((3, 1))
        self.ee_rot_prev = np.eye(3)
        self.time_prev = self.get_clock().now()
        self.first_callback = True

        # DH model for Jacobian
        link01 = RevoluteMDH(a=0, alpha=0, d=self.L0, offset=0)
        link12 = RevoluteMDH(a=self.L1, alpha=np.pi/2, d=0, offset=np.pi/2)
        link23 = RevoluteMDH(a=0, alpha=np.pi/2, d=0, offset=0)
        self.robot = DHRobot([link01, link12, link23], name='RIP')
        self.robot.tool = SE3.Tx(self.L2)

        # Update timer
        self.timer = self.create_timer(0.001, self.update)
        
        self.get_logger().info('Theory Joint Publisher started')
        
    def rotation_matrix_to_angular_velocity(self, R_current, R_prev, dt):
        """Compute angular velocity from two rotation matrices"""
        if dt <= 0:
            return np.zeros(3)
        
        R_diff = R_current @ R_prev.T
        skew = (R_diff - np.eye(3)) / dt
        
        wx = (skew[2, 1] - skew[1, 2]) / 2
        wy = (skew[0, 2] - skew[2, 0]) / 2
        wz = (skew[1, 0] - skew[0, 1]) / 2
        
        return np.array([wx, wy, wz])
    
    def js_callback(self, msg: JointState):
        # Extract encoder data
        if len(msg.position) >= 2:
            q1_new = msg.position[0]
            q2_new = msg.position[1]
        else:
            return
        
        if len(msg.velocity) >= 2:
            self.dq1_measured = msg.velocity[0]
            self.dq2_measured = msg.velocity[1]
        
        time_now = self.get_clock().now()
        
        # Forward kinematics
        q = np.array([q1_new, 0.0, q2_new])
        T = self.robot.fkine(q)
        ee_pos_new = T.t.reshape(3, 1)
        ee_rot_new = T.R
        
        if not self.first_callback:
            dt = (time_now - self.time_prev).nanoseconds / 1e9
            
            if dt > 0:
                # Differentiate FK to get end-effector velocity (THEORY)
                v_ee_theory = (ee_pos_new - self.ee_pos_prev) / dt
                w_ee_theory = self.rotation_matrix_to_angular_velocity(
                    ee_rot_new, self.ee_rot_prev, dt
                ).reshape(3, 1)
                
                # Get Jacobian
                J = self.robot.jacob0(q)
                J_actual = J[:, [0, 2]]
                
                # Use inverse Jacobian to get theory joint velocities
                v_ee_theory_full = np.vstack([v_ee_theory, w_ee_theory])
                J_pinv = np.linalg.pinv(J_actual)
                dq_theory = J_pinv @ v_ee_theory_full
                
                self.dq1_theory = float(dq_theory[0, 0])
                self.dq2_theory = float(dq_theory[1, 0])
            else:
                self.dq1_theory = 0.0
                self.dq2_theory = 0.0
        else:
            self.dq1_theory = 0.0
            self.dq2_theory = 0.0
            self.first_callback = False
    
        self.ee_pos_prev = ee_pos_new
        self.ee_rot_prev = ee_rot_new
        self.time_prev = time_now
        
        self.q1 = q1_new
        self.q2 = q2_new

    def update(self):
        # Compute end-effector velocity using measured joint velocities
        q = np.array([self.q1, 0.0, self.q2])
        J = self.robot.jacob0(q)
        J_actual = J[:, [0, 2]]
        
        dq_measured = np.array([[self.dq1_measured], [self.dq2_measured]])
        v_ee_full = J_actual @ dq_measured
        
        v_ee = v_ee_full[:3]
        w_ee = v_ee_full[3:]
        v_ee_mag = float(np.linalg.norm(v_ee))
        w_ee_mag = float(np.linalg.norm(w_ee))

        # Publish THEORY joint velocities (for comparison)
        js = JointState()
        js.header = Header()
        js.header.stamp = self.get_clock().now().to_msg()
        js.name = [NAME_MAP[k] for k in ORDER_IN]
        js.position = [float(self.q1), float(self.q2)]
        js.velocity = [float(self.dq1_theory), float(self.dq2_theory)]
        js.effort = []
        self.js_pub.publish(js)

        # Publish end-effector velocity
        ee_msg = Float32MultiArray()
        ee_msg.data = [v_ee_mag, w_ee_mag]
        self.ee_pub.publish(ee_msg)

        # TF broadcasting
        x1 = self.arm_joint_x
        y1 = self.arm_joint_y
        z1 = self.arm_joint_z
        
        rot_arm = R.from_euler('z', self.q1)
        quat_arm = rot_arm.as_quat()
        
        x2 = self.arm_joint_x + (self.pend_joint_x_local * math.cos(self.q1) - 
                                   self.pend_joint_y_local * math.sin(self.q1))
        y2 = self.arm_joint_y + (self.pend_joint_x_local * math.sin(self.q1) + 
                                   self.pend_joint_y_local * math.cos(self.q1))
        z2 = self.arm_joint_z + self.pend_joint_z_local
        
        rot_pend = R.from_euler('ZYZ', [self.q1, np.pi/2, self.q2])
        quat_pend = rot_pend.as_quat()

        t_now = self.get_clock().now().to_msg()

        tf1 = TransformStamped()
        tf1.header.stamp = t_now
        tf1.header.frame_id = 'base_link'
        tf1.child_frame_id = 'arm_link_theory'
        tf1.transform.translation.x = float(x1)
        tf1.transform.translation.y = float(y1)
        tf1.transform.translation.z = float(z1)
        tf1.transform.rotation.x = float(quat_arm[0])
        tf1.transform.rotation.y = float(quat_arm[1])
        tf1.transform.rotation.z = float(quat_arm[2])
        tf1.transform.rotation.w = float(quat_arm[3])

        tf2 = TransformStamped()
        tf2.header.stamp = t_now
        tf2.header.frame_id = 'base_link'
        tf2.child_frame_id = 'pendulum_link_theory'
        tf2.transform.translation.x = float(x2)
        tf2.transform.translation.y = float(y2)
        tf2.transform.translation.z = float(z2)
        tf2.transform.rotation.x = float(quat_pend[0])
        tf2.transform.rotation.y = float(quat_pend[1])
        tf2.transform.rotation.z = float(quat_pend[2])
        tf2.transform.rotation.w = float(quat_pend[3])

        tf3 = TransformStamped()
        tf3.header.stamp = t_now
        tf3.header.frame_id = 'pendulum_link_theory'
        tf3.child_frame_id = 'end_effector_theory'
        tf3.transform.translation.x = float(self.L2)
        tf3.transform.translation.y = 0.0
        tf3.transform.translation.z = 11.5e-3
        tf3.transform.rotation.w = 1.0

        self.tf_pub.sendTransform([tf1, tf2, tf3])


def main():
    rclpy.init()
    node = TheoryJointPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()