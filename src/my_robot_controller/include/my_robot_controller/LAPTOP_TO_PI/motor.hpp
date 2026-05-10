#pragma once

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

//motor노드에서 uart<->토픽 변환역할도 같이 수행

class Motor:public rclcpp::Node
{
  public: 
  Motor();
  virtual ~Motor();

  private:
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_cmd;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_actual_vel;
  rclcpp::TimerBase::SharedPtr timer_;
  int uart_fd;
  int serial_fd;
  void init_serial(); 
  rclcpp::Time last_write_time;
};




