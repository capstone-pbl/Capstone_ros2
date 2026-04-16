#pragma once
#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/twist.hpp"


class Odom:public rclcpp::Node
{
  public:
  Odom();

  private:
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr actual_vel; 
  rclcpp::Time prev_time;
  double x=0.0,y=0.0,theta=0.0;
};





