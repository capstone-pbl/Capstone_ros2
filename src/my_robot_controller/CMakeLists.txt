cmake_minimum_required(VERSION 3.8)
project(my_robot_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

include_directories(include)

add_executable(motor src/LAPTOP_TO_PI/motor.cpp)
ament_target_dependencies(motor rclcpp geometry_msgs)

add_executable(odom src/PI_TO_LAPTOP/odom.cpp)
ament_target_dependencies(odom rclcpp geometry_msgs nav_msgs)

find_package(nav2_msgs REQUIRED)
find_package(rclcpp_action REQUIRED)

add_executable(brain src/LAPTOP_TO_PI/brain.cpp)
ament_target_dependencies(brain rclcpp std_msgs nav2_msgs rclcpp_action)
install(TARGETS
  motor
  odom
  brain
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)




ament_package()
