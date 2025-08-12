import os
import launch
from launch.actions import IncludeLaunchDescription
from launch.actions import GroupAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    package_name = "launch_tutorial"
    launch_dir = get_package_share_directory(package_name)
    cmd_group = GroupAction([
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(launch_dir, 'A_turtlesim_mimic.launch.py')
            ),
        )
    ])
    ld = launch.LaunchDescription()
    ld.add_action(cmd_group)
    return ld