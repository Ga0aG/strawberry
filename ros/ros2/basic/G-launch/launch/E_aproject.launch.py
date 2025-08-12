import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.actions import GroupAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import PushRosNamespace


def generate_launch_description():
   # Declare launch argument
   turtlesim_world_1 = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([os.path.join(
         get_package_share_directory('launch_tutorial'), 'launch'),
         '/E_turtlesim_world_1.launch.py'])
      )
   # Use config file to set argument
   turtlesim_world_2 = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([os.path.join(
         get_package_share_directory('launch_tutorial'), 'launch'),
         '/E_turtlesim_world_2.launch.py'])
      )
   turtlesim_world_3 = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([os.path.join(
         get_package_share_directory('launch_tutorial'), 'launch'),
         '/E_turtlesim_world_3.launch.py'])
      )
   # Change namespace
   turtlesim_world_2_with_namespace = GroupAction(
     actions=[
         PushRosNamespace('turtlesim3'),
         turtlesim_world_3,
      ]
   )
   # Broadcast turtle poses to tf2
   broadcaster_nodes = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([os.path.join(
         get_package_share_directory('launch_tutorial'), 'launch'),
         '/E_tfbroadcaster.launch.py'])
      )
   # broadcast frame carrot1
   fixed_frame_node = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([os.path.join(
         get_package_share_directory('launch_tutorial'), 'launch'),
         '/E_fixed_broadcaster.launch.py'])
      )
   # spawn a <turtle2> in the window of turtle1, and go for the carrot
   listener_node = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([os.path.join(
         get_package_share_directory('launch_tutorial'), 'launch'),
         '/E_listener.launch.py']),
      launch_arguments={'target_frame': 'carrot1'}.items(),
      )
   # tutlesim2/turtle mimic/follow <turtle2>'s trajectory
   mimic_node = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([os.path.join(
         get_package_share_directory('launch_tutorial'), 'launch'),
         '/E_mimic.launch.py'])
      )
   rviz_node = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([os.path.join(
         get_package_share_directory('launch_tutorial'), 'launch'),
         '/E_turtlesim_rviz.launch.py'])
      )

   return LaunchDescription([
      turtlesim_world_1,
      turtlesim_world_2,
      turtlesim_world_2_with_namespace,
      broadcaster_nodes,
      listener_node,
      mimic_node,
      fixed_frame_node,
      rviz_node
   ])