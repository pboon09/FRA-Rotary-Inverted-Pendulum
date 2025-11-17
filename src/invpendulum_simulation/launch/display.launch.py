#!/usr/bin/env python3
import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, TimerAction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')

    # --- Paths ---
    urdf_dir = get_package_share_directory('invpendulum_description')
    urdf_path = os.path.join(urdf_dir, 'urdf', 'Rotary_Inverted_Pendulum.urdf')

    rviz_dir = get_package_share_directory('invpendulum_simulation')
    default_rviz_config = os.path.join(rviz_dir, 'rviz', 'rip_display.rviz')

    # --- Args ---
    rvizconfig_arg = DeclareLaunchArgument(
        'rvizconfig', default_value=default_rviz_config,
        description='Path to an RViz config (.rviz)')
    use_sim_time_arg = DeclareLaunchArgument('use_sim_time', default_value='false')

    # --- Load URDF ---
    with open(urdf_path, 'r') as f:
        urdf_xml = f.read()

    # Robot State Publisher
    rsp = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[
            {'robot_description': urdf_xml},
            {'publish_frequency': 50.0},
            {'use_sim_time': use_sim_time}
        ],
    )

    # Static TF: world -> base_link
    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_world_to_base',
        output='screen',
        arguments=['0','0','0','0','0','0','world','base_link'],
    )

    # Joint State Republisher: joint_states_raw -> joint_states
    repub = Node(
        package='invpendulum_simulation',
        executable='joint_state_republisher.py',
        name='joint_state_republisher',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
    )

    # Theory Joint State Publisher
    theory_pub = Node(
        package='invpendulum_simulation',
        executable='theory_joint_state_publisher.py',
        name='theory_joint_state_publisher',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
    )

    # Velocity HUD
    velocity_hud = Node(
        package='invpendulum_simulation',
        executable='velocity_hud.py',
        name='velocity_hud',
        output='screen',
        parameters=[{
            'text_height': 0.03,
            'offset_xyz': [0.0, 0.0, 0.45],
            'color_rgba': [1.0, 1.0, 1.0, 1.0],
            'use_sim_time': use_sim_time,
        }]
    )

    # RViz
    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', LaunchConfiguration('rvizconfig')],
        parameters=[{'use_sim_time': use_sim_time}],
    )

    # Seed joint_states_raw once (before MCU connects)
    seed_once = TimerAction(
        period=0.2,
        actions=[ExecuteProcess(
            cmd=[
                'ros2', 'topic', 'pub', '--once',
                '/joint_states_raw', 'sensor_msgs/JointState',
                "{name: ['joint_base_to_arm','joint_arm_to_pendulum'], position: [0.0, 0.0], velocity: [0.0, 0.0]}"
            ],
            output='screen'
        )]
    )

    return LaunchDescription([
        use_sim_time_arg,
        rvizconfig_arg,
        rsp,
        static_tf,
        repub,
        theory_pub,
        velocity_hud,
        seed_once,  
        rviz, 
    ])