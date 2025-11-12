#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from std_msgs.msg import Header

# Map micro-ROS names -> URDF joint names
NAME_MAP = {
    'joint_base_to_arm': 'Rev_Arm',
    'joint_arm_to_pendulum': 'Rev_Pendulum',
}
ORDER_IN = ['joint_base_to_arm', 'joint_arm_to_pendulum']  # expected incoming order

class JointStateRepublisher(Node):
    def __init__(self):
        super().__init__('joint_state_republisher')
        self.subscription = self.create_subscription(
            JointState, 'joint_states_raw', self.joint_state_callback, 10
        )
        self.publisher = self.create_publisher(JointState, 'joint_states', 10)
        self.get_logger().info('Joint State Republisher started: joint_states_raw -> joint_states')

    def joint_state_callback(self, msg: JointState):
        out = JointState()
        out.header = Header()
        out.header.stamp = self.get_clock().now().to_msg()

        name2pos = dict(zip(msg.name, msg.position))
        name2vel = dict(zip(msg.name, msg.velocity)) if len(msg.velocity) == len(msg.name) else {}

        out.name = [NAME_MAP.get(k, k) for k in ORDER_IN]
        out.position = [name2pos.get(k, 0.0) for k in ORDER_IN]
        out.velocity = [name2vel.get(k, 0.0) for k in ORDER_IN]
        out.effort = []

        self.publisher.publish(out)

def main(args=None):
    rclpy.init(args=args)
    node = JointStateRepublisher()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
