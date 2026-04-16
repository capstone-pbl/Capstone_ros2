
// 라이다에서 측정한 데이터(pi)->laptop

#pragma once
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"


class Laser_Scan:public rclcpp::Node
{
 public:
 Laser_Scan();

 private:
 rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr pub_;
 //필터링 한 후의 데이터
 rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr sub_;
 //라이다 드라이버에서 scan토픽으로 받아오는 '필터링되기전의 데이터'
}



