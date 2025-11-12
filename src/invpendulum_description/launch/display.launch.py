from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, Command
from launch_ros.substitutions import FindPackageShare
from launch_ros.actions import Node
from launch.conditions import IfCondition


# 'ros2 launch invpendulum_description display.launch.py use_gui:=true' for using gui
# 'ros2 launch invpendulum_description display.launch.py use_gui:=false' for using real joint state

def generate_launch_description():
    ld = LaunchDescription()

    # Package path
    pkg_path = FindPackageShare('invpendulum_description')

    # File paths
    default_model_path = PathJoinSubstitution([pkg_path, 'urdf', 'Rotary-Inverted-Pendulum.xacro'])
    default_rviz_config_path = PathJoinSubstitution([pkg_path, 'rviz', 'rviz.rviz'])
    urdf_launch_path = PathJoinSubstitution([FindPackageShare('urdf_launch'), 'launch', 'display.launch.py'])

    # Arguments
    use_gui_arg = DeclareLaunchArgument(
        name='use_gui',
        default_value='false',
        choices=['true', 'false'],
        description='Use joint_state_publisher_gui (true) or external /joint_states (false)'
    )

    model_arg = DeclareLaunchArgument(
        name='model',
        default_value=default_model_path,
        description='Path to the robot Xacro file'
    )

    rviz_arg = DeclareLaunchArgument(
        name='rvizconfig',
        default_value=default_rviz_config_path,
        description='Absolute path to the RViz config file'
    )

    # Convert Xacro to URDF at runtime
    robot_description = Command(['xacro ', LaunchConfiguration('model')])

    # Include urdf_launch display (handles GUI internally)
    include_display = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(urdf_launch_path),
        launch_arguments={
            'urdf_package': 'invpendulum_description',
            'urdf_package_path': LaunchConfiguration('model'),
            'rviz_config': LaunchConfiguration('rvizconfig'),
            'jsp_gui': LaunchConfiguration('use_gui')
        }.items()
    )

    # Add all actions
    ld.add_action(use_gui_arg)
    ld.add_action(model_arg)
    ld.add_action(rviz_arg)
    ld.add_action(include_display)

    return ld
