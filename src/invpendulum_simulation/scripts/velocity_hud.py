#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from visualization_msgs.msg import Marker
from sensor_msgs.msg import JointState

URDF_NAMES = ['Rev_Arm', 'Rev_Pendulum']
MCU_NAMES  = ['joint_base_to_arm', 'joint_arm_to_pendulum']

class VelocityHUD(Node):
    def __init__(self):
        super().__init__('velocity_hud')

        self.declare_parameter('text_height', 0.05)
        self.declare_parameter('offset_xyz', [0.0, 0.0, 0.5])
        self.declare_parameter('color_rgba', [1.0, 1.0, 1.0, 1.0])  # white

        self.frame_id = 'base_link'
        self.text_h = float(self.get_parameter('text_height').value)
        off = self.get_parameter('offset_xyz').value
        col = self.get_parameter('color_rgba').value
        self.offx, self.offy, self.offz = map(float, off)
        self.colr, self.colg, self.colb, self.cola = map(float, col)

        self.meas_vel = {URDF_NAMES[0]: 0.0, URDF_NAMES[1]: 0.0}
        self.theo_vel = {URDF_NAMES[0]: 0.0, URDF_NAMES[1]: 0.0}

        self.pub = self.create_publisher(Marker, "rip_hud", 10)
        self.sub_meas = self.create_subscription(JointState, "joint_states", self.cb_meas, 10)
        self.sub_theo = self.create_subscription(JointState, "joint_states_theory", self.cb_theo, 10)

        self.create_timer(0.05, self.update_display)


    def cb_meas(self, msg: JointState):

        name2vel = dict(zip(msg.name, msg.velocity)) if len(msg.velocity) == len(msg.name) else {}
        for key in URDF_NAMES + MCU_NAMES:
            if key in name2vel:
                if key in URDF_NAMES:
                    self.meas_vel[key] = name2vel[key]
                elif key == MCU_NAMES[0]:
                    self.meas_vel[URDF_NAMES[0]] = name2vel[key]
                elif key == MCU_NAMES[1]:
                    self.meas_vel[URDF_NAMES[1]] = name2vel[key]

    def cb_theo(self, msg: JointState):

        name2vel = dict(zip(msg.name, msg.velocity)) if len(msg.velocity) == len(msg.name) else {}
        for key in URDF_NAMES + MCU_NAMES:
            if key in name2vel:
                if key in URDF_NAMES:
                    self.theo_vel[key] = name2vel[key]
                elif key == MCU_NAMES[0]:
                    self.theo_vel[URDF_NAMES[0]] = name2vel[key]
                elif key == MCU_NAMES[1]:
                    self.theo_vel[URDF_NAMES[1]] = name2vel[key]

    def update_display(self):

        arm_m = self.meas_vel[URDF_NAMES[0]]
        pend_m = self.meas_vel[URDF_NAMES[1]]
        arm_t = self.theo_vel[URDF_NAMES[0]]
        pend_t = self.theo_vel[URDF_NAMES[1]]

        text = (
            f"Arm velocity meas: {arm_m:+.3f} rad/s\n"
            f"Pendulum velocity meas: {pend_m:+.3f} rad/s\n"
            f"Arm velocity theory: {arm_t:+.3f} rad/s\n"
            f"Pendulum velocity theory: {pend_t:+.3f} rad/s"
        )

        m = Marker()
        m.header.frame_id = self.frame_id
        m.header.stamp = self.get_clock().now().to_msg()
        m.ns = "rip_hud"
        m.id = 1
        m.type = Marker.TEXT_VIEW_FACING
        m.action = Marker.ADD

        m.pose.position.x = self.offx
        m.pose.position.y = self.offy
        m.pose.position.z = self.offz
        m.pose.orientation.w = 1.0

        # Compact text scaling
        m.scale.z = self.text_h
        m.scale.x = self.text_h * 0.6
        m.scale.y = 0.01

        # Color
        m.color.r, m.color.g, m.color.b, m.color.a = self.colr, self.colg, self.colb, self.cola
        m.text = text
        m.lifetime.sec = 0

        self.pub.publish(m)


def main():
    rclpy.init()
    node = VelocityHUD()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
