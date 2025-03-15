from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='turtlesim',
            namespace='turtlesim1',
            executable='turtlesim_node',
            name='sim',
            parameters=[{'window_title': 'turtle1'}]
        ),
        Node(
            package='turtlesim',
            namespace='turtlesim2',
            executable='turtlesim_node',
            name='sim',
            parameters=[{'window_title': 'turtle2'}],
        ),
        Node(
            package='turtlesim',
            executable='mimic',
            name='mimic',
            remappings=[
                ('/input/pose', '/turtlesim1/turtle1/pose'),
                ('/output/cmd_vel', '/turtlesim2/turtle1/cmd_vel'),
            ]
        ),
        # Node(
        #     package='turtlesim',
        #     executable='turtle_teleop_key',
        #     emulate_tty=True,
        #     output="both",
        #     # parameters=[{'use_sim_time': False}],
        #     remappings=[
        #         ("turtle1/cmd_vel", "/turtlesim1/turtle1/cmd_vel")
        #     ],
        # )
    ])