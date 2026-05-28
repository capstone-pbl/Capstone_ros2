#pragma once
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include <map>
#include <string>
#include <algorithm>
class Brain:public rclcpp::Node
{
 public:
 Brain();



 private:
 std::map<std::string,std::pair<double,double>>zone_coords;
 rclcpp::Subscription<std_msgs::msg::String>::SharedPtr calling_;
 rclcpp::TimerBase::SharedPtr timer_;
 rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr nav_client_;
 rclcpp::Publisher<std_msgs::msg::String>::SharedPtr state_pub;
 enum State
 {
  IDLE , MOVING,ARRIVED,RETURNING

 };
 State current_state = IDLE;
 std::string zone_name;

};