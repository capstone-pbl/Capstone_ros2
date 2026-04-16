#include "rclcpp/rclcpp.hpp"
#include "my_robot_controller/PI_TO_LAPTOP/odom.hpp"
#include <cmath>


 Odom::Odom():rclcpp::Node("Odom")
{

  
  prev_time=this->now();
  odom_ = this->create_publisher<nav_msgs::msg::Odometry>("/odom",10);
  //엔코더피드백을 받아서 자기위치추정해서 odom토픽으로 퍼블리시

  actual_vel=this->create_subscription<geometry_msgs::msg::Twist>
  ("/actual_vel",10,[this](geometry_msgs::msg::Twist::SharedPtr msg)
  {
    
    rclcpp::Time now = this->now();          
    double dt = (now-prev_time).seconds();
   
    

    double v = msg->linear.x;
    double w = msg->angular.z;
   
    theta += w*dt; 
    x += v*cos(theta)*dt;
    y += v*sin(theta)*dt;


    auto odom_msg = nav_msgs::msg::Odometry();  // Odometry 메시지 생성

    odom_msg.header.stamp = now;                // 타임스탬프 (언제 측정했는지)
    odom_msg.header.frame_id = "odom";          // 기준 좌표계
    odom_msg.child_frame_id = "base_footprint"; // 로봇 좌표계

    odom_msg.pose.pose.position.x = x;         // 로봇 현재 x 위치
    odom_msg.pose.pose.position.y = y;         // 로봇 현재 y 위치

    odom_msg.twist.twist.linear.x = v;         // 현재 선속도
    odom_msg.twist.twist.angular.z = w;        // 현재 각속도
    odom_->publish(odom_msg);
    prev_time = now;
  });   //actual_vel로 현재 위치를 추정한 데이터를 가공해서 slam으로 보내야함
  
      
}




int main(int argc,char ** argv)
{
rclcpp::init(argc,argv);
auto node=std::make_shared<Odom>();
rclcpp::spin(node);
rclcpp::shutdown();
}