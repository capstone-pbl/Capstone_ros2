


#pragma once
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

class Imu:public rclcpp::Node
{
 public:
    Imu();

 private:
  rclcpp::Publisher<sensor_msgs::msg::imu>::SharedPtr Pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};


