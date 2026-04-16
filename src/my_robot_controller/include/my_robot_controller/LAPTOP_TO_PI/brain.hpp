#pragma once
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class Brain:public rclcpp::Node
{
 public:
 Brain();



 private:

 rclcpp::Subscription<std_msgs::msg::String>::SharedPtr calling_;
 rclcpp::TimerBase::SharedPtr timer_;
 enum State
 {
  IDLE , MOVING,ARRIVED,RETURNING

 };
 State current_state = IDLE;
 std::string zone_;
};