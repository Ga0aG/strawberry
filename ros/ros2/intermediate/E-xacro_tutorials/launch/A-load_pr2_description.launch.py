from launch import LaunchDescription
from launch.substitutions import Command
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory, get_package_share_path

def generate_launch_description():
    path_to_urdf = get_package_share_path('pr2_description') / 'robots' / 'pr2.urdf.xacro'
    return LaunchDescription([
        Node(package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{
                'robot_description': ParameterValue(
                    Command(['xacro ', str(path_to_urdf)]), value_type=str
                )
            }]),
    ])
