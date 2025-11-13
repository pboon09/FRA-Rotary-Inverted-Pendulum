#!/usr/bin/env python3
import math
import numpy as np
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from std_msgs.msg import Header
from geometry_msgs.msg import TransformStamped
from tf2_ros import TransformBroadcaster
from roboticstoolbox import DHRobot, RevoluteMDH
from spatialmath import SE3
from invpendulum_simulation.joint_name_config import NAME_MAP, ORDER_IN


class TheoryJointPublisher(Node):
    def __init__(self):
        super().__init__('theory_joint_publisher')

        # Publishers
        self.js_pub = self.create_publisher(JointState, 'joint_states_theory', 10)
        self.tf_pub = TransformBroadcaster(self)
        
        # Subscription
        self.create_subscription(JointState, "joint_states_raw", self.js_raw_callback, 10)
        
        # Physical parameters (m)
        self.L1 = 240.40e-3
        self.L2 = 255.46e-3
        self.L0 = 64e-3

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
        
        q1 = name2pos.get(ORDER_IN[0], 0.0)
        q2 = name2pos.get(ORDER_IN[1], 0.0)
        dq1 = name2vel.get(ORDER_IN[0], 0.0)
        dq2 = name2vel.get(ORDER_IN[1], 0.0)
        
        L1, L2 = self.L1, self.L2
        
        x1 = L1 * math.cos(q1)
        y1 = L1 * math.sin(q1)
        z1 = 0.0
        
        x2 = L1 * math.cos(q1) - L2 * math.sin(q2) * math.sin(q1)
        y2 = L1 * math.sin(q1) + L2 * math.sin(q2) * math.cos(q1)
        z2 = L2 * math.cos(q2)
        
        # Optional: Calculate Jacobian and end-effector velocity
        q = np.array([q1, 0.0, q2])
        dq = np.array([dq1, 0.0, dq2])
        J = self.robot.jacob0(q)
        J_actual = J[:, [0, 2]]
        v_ee = J_actual[:3, :] @ np.array([[dq1], [dq2]])
        w_ee = J_actual[3:, :] @ np.array([[dq1], [dq2]])
        
        # Publish TF transforms for ACTUAL robot (from micro-ROS data)
        t_now = self.get_clock().now().to_msg()
        
        tf1 = TransformStamped()
        tf1.header.stamp = t_now
        tf1.header.frame_id = 'base_link'
        tf1.child_frame_id = 'arm_link1_actual'
        tf1.transform.translation.x = float(x1)
        tf1.transform.translation.y = float(y1)
        tf1.transform.translation.z = float(z1)
        tf1.transform.rotation.w = 1.0
        
        tf2 = TransformStamped()
        tf2.header.stamp = t_now
        tf2.header.frame_id = 'base_link'
        tf2.child_frame_id = 'pendulum_link2_actual'
        tf2.transform.translation.x = float(x2)
        tf2.transform.translation.y = float(y2)
        tf2.transform.translation.z = float(z2)
        tf2.transform.rotation.w = 1.0
        
        self.tf_pub.sendTransform([tf1, tf2])
        
        self.get_logger().debug(
            f"Actual FK: q1={q1:+.3f} q2={q2:+.3f} | "
            f"Link1({x1:.3f},{y1:.3f},{z1:.3f}) Link2({x2:.3f},{y2:.3f},{z2:.3f})"
        )
        
        

    def update(self):

        q1 = 0.5 * math.sin(0.5 * self.t)
        q2 = 0.2 * math.cos(1.0 * self.t)
        dq1 = 0.25 * math.cos(0.5 * self.t)
        dq2 = -0.2 * math.sin(1.0 * self.t)

        L1, L2 = self.L1, self.L2
        x1 = L1 * math.cos(q1)
        y1 = L1 * math.sin(q1)
        z1 = 0.0

        x2 = L1 * math.cos(q1) - L2 * math.sin(q2) * math.sin(q1)
        y2 = L1 * math.sin(q1) + L2 * math.sin(q2) * math.cos(q1)
        z2 = L2 * math.cos(q2)

        q = np.array([q1, 0.0, q2])
        dq = np.array([dq1, 0.0, dq2])
        J = self.robot.jacob0(q)
        J_actual = J[:, [0, 2]]
        v_ee = J_actual[:3, :] @ np.array([[dq1], [dq2]])
        w_ee = J_actual[3:, :] @ np.array([[dq1], [dq2]])

        js = JointState()
        js.header = Header()
        js.header.stamp = self.get_clock().now().to_msg()
        js.name = [NAME_MAP[k] for k in ORDER_IN]
        js.position = [float(q1), float(q2)]
        js.velocity = [float(dq1), float(dq2)]
        js.effort = []
        self.js_pub.publish(js)

        t_now = self.get_clock().now().to_msg()

        tf1 = TransformStamped()
        tf1.header.stamp = t_now
        tf1.header.frame_id = 'base_link'
        tf1.child_frame_id = 'arm_link1'
        tf1.transform.translation.x = float(x1)
        tf1.transform.translation.y = float(y1)
        tf1.transform.translation.z = float(z1)
        tf1.transform.rotation.w = 1.0

        tf2 = TransformStamped()
        tf2.header.stamp = t_now
        tf2.header.frame_id = 'base_link'
        tf2.child_frame_id = 'pendulum_link2'
        tf2.transform.translation.x = float(x2)
        tf2.transform.translation.y = float(y2)
        tf2.transform.translation.z = float(z2)
        tf2.transform.rotation.w = 1.0

        self.tf_pub.sendTransform([tf1, tf2])

        # optional debug
        self.get_logger().debug(
            f"t={self.t:.2f}  q1={q1:+.3f}  q2={q2:+.3f} | "
            f"Link1({x1:.3f},{y1:.3f},{z1:.3f})  Link2({x2:.3f},{y2:.3f},{z2:.3f})"
        )

        self.t += self.dt


def main():
    rclpy.init()
    node = TheoryJointPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
