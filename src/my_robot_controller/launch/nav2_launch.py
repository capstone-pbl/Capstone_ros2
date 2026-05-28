from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')

    map_arg = DeclareLaunchArgument('map', default_value='')
    map_path = LaunchConfiguration('map')
    map_path_no_ext = PythonExpression(["'", map_path, "'.replace('.yaml', '')"])

    return LaunchDescription([
        map_arg,

        # 라이다 TF
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            arguments=['--x', '0', '--y', '0', '--z', '0',
                       '--roll', '0', '--pitch', '0', '--yaw', '3.14159',
                       '--frame-id', 'base_footprint', '--child-frame-id', 'laser']
        ),

        # base_link TF
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            arguments=['--x', '0', '--y', '0', '--z', '0',
                       '--roll', '0', '--pitch', '0', '--yaw', '0',
                       '--frame-id', 'base_footprint', '--child-frame-id', 'base_link']
        ),

        # Foxglove
        Node(
            package='foxglove_bridge',
            executable='foxglove_bridge',
            parameters=[{'port': 8765}]
        ),

        # map_server
        Node(
            package='nav2_map_server',
            executable='map_server',
            name='map_server',
            output='screen',
            parameters=[{
                'yaml_filename': map_path,
                'use_sim_time': False
            }]
        ),

        # SLAM Toolbox localization
        Node(
            package='slam_toolbox',
            executable='localization_slam_toolbox_node',
            name='slam_toolbox',
            output='screen',
            remappings=[('/map', '/slam_map')], 
            parameters=[
                os.path.expanduser('~/nav2_params.yaml'),
                {
                    'use_sim_time': False,
                    'map_file_name': map_path_no_ext,
                    'mode': 'localization',
                    'map_start_at_dock': False
                }
            ]
        ),

        # Navigation (AMCL 없음)
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(nav2_bringup_dir, 'launch', 'navigation_launch.py')
            ),
            launch_arguments={
                'use_sim_time': 'false',
                'autostart': 'true',
                'params_file': os.path.expanduser('~/nav2_params.yaml')
            }.items()
        ),
    ])
