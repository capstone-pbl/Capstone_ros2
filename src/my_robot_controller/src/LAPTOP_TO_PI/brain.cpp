#include "my_robot_controller/LAPTOP_TO_PI/brain.hpp"
#include <string>


Brain::Brain():Node("Brain")
{

calling_ = this->create_subscription<std_msgs::msg::String>("/call_zone",10,
[this](std_msgs::msg::String::SharedPtr msg)
{
 
 zone_ = msg -> data;
 RCLCPP_INFO(this->get_logger(),"호출: %s 구역", zone_.c_str());
 this->current_state=MOVING;


}
);


timer_ = this->create_wall_timer(std::chrono::milliseconds(100),
[this]()
{

switch(current_state)
{
 case IDLE:
 {  
  static int count=0;
  if((count++ % 50)==0)RCLCPP_INFO(this->get_logger(),"[IDLE]명령 대기중");

 break;
 }
 
 case MOVING:
 
 {
 if((count++ % 5)==0)RCLCPP_INFO(this->get_logger(),"[MOVING]%s구역으로 이동중",zone_.c_str());
 
 break;
 
 }
 case ARRIVED:
 {
 
 if((count++ % 5)==0)RCLCPP_INFO(this->get_logger(),"[ARRIVED]%s구역에 도착. 사용자 탐색중",zone_.c_str());
 
 break;
 
 }
 case RETURNING:
 
 {
 if((count++ % 5)==0)RCLCPP_INFO(this->get_logger(),"[RETURNING]%s구역 탐색 마치고 복귀중",zone_.c_str());
 
 break;

 }

}



}




);



}

int main(int argc,char ** argv)
{
  rclcpp::init(argc,argv);
  auto node=std::make_shared<Brain>();
  rclcpp::spin(node);
  rclcpp::shutdown();


    return 0;
}