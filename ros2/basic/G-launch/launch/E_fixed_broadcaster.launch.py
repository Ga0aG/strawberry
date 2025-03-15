from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import EnvironmentVariable, LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
   #  Environment variables can be used to define or push namespaces for distinguishing nodes on different computers or robots.
   return LaunchDescription([
      DeclareLaunchArgument(
            'node_prefix',
            # default_value=[EnvironmentVariable('USER'), '_'],
            default_value=["bot"],
            description='prefix for node name'
      ),
      Node(
            package='turtle_tf2_py',
            executable='fixed_frame_tf2_broadcaster',
            name=[LaunchConfiguration('node_prefix'), 'fixed_broadcaster'],
      ),
   ])