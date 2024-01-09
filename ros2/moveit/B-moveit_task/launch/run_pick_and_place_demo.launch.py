import os
import yaml
import xacro
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

# ros2 launch moveit2_tutorials demo.launch.py
# In Rviz
# I. Add moveit_task_constructor_visualization/Motion Planning Tasks
# II. Change task solution topic under `Motion Planning Tasks` to /solution
# III. Click Exec to execute(Failed)

def load_yaml(package_name, file_path):
    package_path = get_package_share_directory(package_name)
    absolute_file_path = os.path.join(package_path, file_path)

    try:
        with open(absolute_file_path, "r") as file:
            return yaml.safe_load(file)
    except EnvironmentError:  # parent of IOError, OSError *and* WindowsError where available
        return None

def generate_launch_description():
    robot_description_config = xacro.process_file(
        os.path.join(
            get_package_share_directory("moveit_resources_panda_moveit_config"),
            "config",
            "panda.urdf.xacro",
        )
    )
    robot_description = {"robot_description": robot_description_config.toxml()}
    kinematics_yaml = load_yaml(
        "moveit_resources_panda_moveit_config", "config/kinematics.yaml"
    )

    # MTC Demo node
    pick_place_demo = Node(
        package="mtc_tutorial",
        executable="pick_and_place_demo",
        output="screen",
        parameters=[
            robot_description,
            kinematics_yaml
        ],
    )

    return LaunchDescription([pick_place_demo])