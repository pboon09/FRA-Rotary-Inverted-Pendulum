#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from visualization_msgs.msg import Marker, MarkerArray
from sensor_msgs.msg import JointState
from std_msgs.msg import Float32MultiArray
from invpendulum_simulation.joint_name_config import NAME_MAP, ORDER_IN


class VelocityHUD(Node):
    def __init__(self):
        super().__init__('velocity_hud')

        self.declare_parameter('text_height', 0.03)
        self.declare_parameter('offset_xyz', [0.0, 0.0, 0.55])
        self.declare_parameter('color_rgba', [1.0, 1.0, 1.0, 1.0])  # white

        self.frame_id = 'base_link'
        self.text_h = float(self.get_parameter('text_height').value)
        off = self.get_parameter('offset_xyz').value
        col = self.get_parameter('color_rgba').value
        self.offx, self.offy, self.offz = map(float, off)
        self.colr, self.colg, self.colb, self.cola = map(float, col)

        self.urdf_names = [NAME_MAP[k] for k in ORDER_IN]
        
        # Joint velocities: measured (from encoders) vs theory (from FK differentiation)
        self.meas_vel = {self.urdf_names[0]: 0.0, self.urdf_names[1]: 0.0}
        self.theo_vel = {self.urdf_names[0]: 0.0, self.urdf_names[1]: 0.0}
        
        # End-effector velocity (only one - calculated from measured joint velocities)
        self.ee_linear_vel = 0.0
        self.ee_angular_vel = 0.0

        # Publishers
        self.pub = self.create_publisher(Marker, "rip_hud", 10)
        self.marker_pub = self.create_publisher(MarkerArray, "frame_markers", 10)
        
        # Subscriptions
        self.sub_meas = self.create_subscription(JointState, "joint_states", self.cb_meas, 10)
        self.sub_theo = self.create_subscription(JointState, "joint_states_theory", self.cb_theo, 10)
        self.sub_ee = self.create_subscription(Float32MultiArray, "end_effector_velocity", self.cb_ee, 10)

        self.create_timer(0.001, self.update_display)

    def cb_meas(self, msg: JointState):
        name2vel = dict(zip(msg.name, msg.velocity)) if len(msg.velocity) == len(msg.name) else {}
        for urdf_name in self.urdf_names:
            if urdf_name in name2vel:
                self.meas_vel[urdf_name] = name2vel[urdf_name]

    def cb_theo(self, msg: JointState):
        name2vel = dict(zip(msg.name, msg.velocity)) if len(msg.velocity) == len(msg.name) else {}
        for urdf_name in self.urdf_names:
            if urdf_name in name2vel:
                self.theo_vel[urdf_name] = name2vel[urdf_name]

    def cb_ee(self, msg: Float32MultiArray):
        if len(msg.data) >= 2:
            self.ee_linear_vel = msg.data[0]
            self.ee_angular_vel = msg.data[1]

    def create_sphere_marker(self, frame_id, r, g, b, marker_id):
        m = Marker()
        m.header.frame_id = frame_id
        from builtin_interfaces.msg import Time
        m.header.stamp = Time()
        m.ns = "frame_spheres"
        m.id = marker_id
        m.type = Marker.SPHERE
        m.action = Marker.ADD
        
        m.pose.position.x = 0.0
        m.pose.position.y = 0.0
        m.pose.position.z = 0.0
        m.pose.orientation.w = 1.0
        
        m.scale.x = 0.04
        m.scale.y = 0.04
        m.scale.z = 0.04
        
        m.color.r = r
        m.color.g = g
        m.color.b = b
        m.color.a = 0.6
        
        m.lifetime.sec = 0
        return m

    def update_display(self):
        """Update HUD display and frame markers"""
        arm_m = self.meas_vel[self.urdf_names[0]]
        pend_m = self.meas_vel[self.urdf_names[1]]
        arm_t = self.theo_vel[self.urdf_names[0]]
        pend_t = self.theo_vel[self.urdf_names[1]]

        # Calculate differences for validation
        arm_diff = abs(arm_m - arm_t)
        pend_diff = abs(pend_m - pend_t)

        # Create text display
        text = (
            f"Arm Joint Velocity (Meas): {arm_m:+.3f} rad/s\n"
            f"Arm Joint Velocity (Theory): {arm_t:+.3f} rad/s\n"
            f"\n"
            f"Pendulum Joint Velocity (Meas): {pend_m:+.3f} rad/s\n"
            f"Pendulum Joint Velocity (Theory): {pend_t:+.3f} rad/s\n"
            f"\n"
            f"End Effector Linear Velocity: {self.ee_linear_vel:.4f} m/s\n"
            f"End Effector Angular Velocity: {self.ee_angular_vel:.4f} rad/s"
        )

        # Text HUD marker
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

        m.scale.z = self.text_h
        m.scale.x = self.text_h * 0.6
        m.scale.y = 0.01

        m.color.r, m.color.g, m.color.b, m.color.a = self.colr, self.colg, self.colb, self.cola
        m.text = text
        m.lifetime.sec = 0

        self.pub.publish(m)

        # Frame markers - simple spheres
        marker_array = MarkerArray()
        
        # Theory frames - PURPLE
        marker_array.markers.append(
            self.create_sphere_marker('arm_link_theory', 0.5, 0.0, 1.0, 1)
        )
        marker_array.markers.append(
            self.create_sphere_marker('pendulum_link_theory', 0.5, 0.0, 1.0, 2)
        )
        marker_array.markers.append(
            self.create_sphere_marker('end_effector_theory', 0.5, 0.0, 1.0, 3)
        )
        
        # URDF frames - YELLOW
        marker_array.markers.append(
            self.create_sphere_marker('arm_link', 1.0, 1.0, 0.0, 4)
        )
        marker_array.markers.append(
            self.create_sphere_marker('pendulum_link', 1.0, 1.0, 0.0, 5)
        )
        marker_array.markers.append(
            self.create_sphere_marker('end_effector', 1.0, 1.0, 0.0, 6)
        )
        
        self.marker_pub.publish(marker_array)


def main():
    rclpy.init()
    node = VelocityHUD()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()