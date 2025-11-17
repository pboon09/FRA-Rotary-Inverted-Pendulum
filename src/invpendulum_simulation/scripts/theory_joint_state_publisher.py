#!/usr/bin/env python3
import math
import numpy as np
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from std_msgs.msg import Header, Float32
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
        self.ee_pub = self.create_publisher(Float32, 'end_effector_theory', 10)
        self.tf_pub = TransformBroadcaster(self)
        
        # Subscription
        self.create_subscription(JointState, "joint_states_raw", self.js_raw_callback, 10)
        
        # Physical parameters FROM URDF (m)
        # Rev_Arm joint origin in base_link
        self.arm_joint_x = 0.0002
        self.arm_joint_y = 0.0
        self.arm_joint_z = 0.212
        
        # Rev_Pendulum joint origin in Arm_Link1 frame
        self.pend_joint_x_local = 0.1215
        self.pend_joint_y_local = 0.0
        self.pend_joint_z_local = 0.0255
        
        # Physical parameters (m)
        self.L1 = 240.40e-3
        self.L2 = 135e-3
        self.L0 = 64e-3
        
        self.q1 = 0.0
        self.q2 = 0.0
        self.dq1 = 0.0
        self.dq2 = 0.0

        # DH model for Jacobian
        link01 = RevoluteMDH(a=0, alpha=0, d=self.L0, offset=0)
        link12 = RevoluteMDH(a=self.L1, alpha=np.pi/2, d=0, offset=np.pi/2)
        link23 = RevoluteMDH(a=0, alpha=np.pi/2, d=0, offset=0)
        self.robot = DHRobot([link01, link12, link23], name='RIP')
        self.robot.tool = SE3.Tx(self.L2)

        # Simulation loop
        self.t = 0.0
        self.dt = 0.02  # 50 Hz
        self.timer = self.create_timer(self.dt, self.update)

        self.get_logger().info("Theoretical Joint + TF publisher started.")
        
    def js_raw_callback(self, msg: JointState):
        
        name2pos = dict(zip(msg.name, msg.position))
        name2vel = dict(zip(msg.name, msg.velocity)) if msg.velocity else {}
        
        self.q1 = name2pos.get(ORDER_IN[0], 0.0)
        self.q2 = name2pos.get(ORDER_IN[1], 0.0)
        self.dq1 = name2vel.get(ORDER_IN[0], 0.0)
        self.dq2 = name2vel.get(ORDER_IN[1], 0.0)

    def update(self):
        # FK based on URDF joint origins
        
        # Arm_Link1 frame: at Rev_Arm joint origin
        x1 = self.arm_joint_x
        y1 = self.arm_joint_y
        z1 = self.arm_joint_z
        
        # Arm_Link1 rotation: rotate q1 about Z axis
        rot_arm = R.from_euler('z', self.q1)
        quat_arm = rot_arm.as_quat()
        
        # Pendulum_Link2 frame: at Rev_Pendulum joint origin
        # Transform from Arm_Link1 frame to base_link frame
        x2 = self.arm_joint_x + (self.pend_joint_x_local * math.cos(self.q1) - 
                                   self.pend_joint_y_local * math.sin(self.q1))
        y2 = self.arm_joint_y + (self.pend_joint_x_local * math.sin(self.q1) + 
                                   self.pend_joint_y_local * math.cos(self.q1))
        z2 = self.arm_joint_z + self.pend_joint_z_local
        
        # Pendulum_Link2 rotation: 
        # 1. Rotate q1 about base Z (arm rotation)
        # 2. Rotate 90° about Y (from URDF rpy="0 1.5708 0")
        # 3. Rotate q2 about the pendulum axis (Z in pendulum frame)
        rot_pend = R.from_euler('ZYZ', [self.q1, np.pi/2, self.q2])
        quat_pend = rot_pend.as_quat()  # [x, y, z, w]

        # Jacobian calculation (using DH model)
        q = np.array([self.q1, 0.0, self.q2])
        dq = np.array([self.dq1, 0.0, self.dq2])
        
        J = self.robot.jacob0(q)
        J_actual = J[:, [0, 2]]
        v_ee = J_actual[:3, :] @ np.array([[self.dq1], [self.dq2]])
        w_ee = J_actual[3:, :] @ np.array([[self.dq1], [self.dq2]])
        v_ee_mag = np.sqrt(v_ee[0]**2 + v_ee[1]**2 + v_ee[2]**2)

        # Publish joint states
        js = JointState()
        js.header = Header()
        js.header.stamp = self.get_clock().now().to_msg()
        js.name = [NAME_MAP[k] for k in ORDER_IN]
        js.position = [float(self.q1), float(self.q2)]
        js.velocity = [float(self.dq1), float(self.dq2)]
        js.effort = []
        self.js_pub.publish(js)

        ee = Float32()
        ee.data = float(v_ee_mag)
        self.ee_pub.publish(ee)

        # Publish TF transforms
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
        tf3.child_frame_id = 'end_effector'
        tf3.transform.translation.x = float(self.L2)
        tf3.transform.translation.y = float(0.0)
        tf3.transform.translation.z = float(11.5e-3)
        tf3.transform.rotation.x = float(0.0)
        tf3.transform.rotation.y = float(0.0)
        tf3.transform.rotation.z = float(0.0)
        tf3.transform.rotation.w = float(1.0)

        self.tf_pub.sendTransform([tf1, tf2, tf3])

        self.t += self.dt



def main():
    rclpy.init()
    node = TheoryJointPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()