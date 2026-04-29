#include "my_robot_controller/LAPTOP_TO_PI/brain.hpp"
#include <string>





Brain::Brain():Node("Brain")
{

nav_client_=rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(this,"navigate_to_pose");
zone_coords["A"]={1.0,2.0};
 zone_coords["B"]={3.0,2.0};
 zone_coords["home"]={0.0,0.0};
calling_ = this->create_subscription<std_msgs::msg::String>("/call_zone",10,
[this](std_msgs::msg::String::SharedPtr msg)
{
 
 zone_name = msg -> data;
 RCLCPP_INFO(this->get_logger(),"호출: %s 구역", zone_name.c_str());
 if(zone_name=="home")this->current_state=RETURNING;
 else this->current_state=MOVING;
 double x,y;
 auto goal = nav2_msgs::action::NavigateToPose::Goal();
 goal.pose.header.frame_id="map";
 goal.pose.header.stamp=this->now();



  auto it=zone_coords.find(zone_name);
  if(it != zone_coords.end())
  {
   x=it->second.first;   
   y=it->second.second;

   goal.pose.pose.position.x= x;
   goal.pose.pose.position.y= y;

    auto send_goal_options = rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SendGoalOptions();
    send_goal_options.result_callback = [this](auto result)
    {
    if(result.code == rclcpp_action::ResultCode::SUCCEEDED)
    {
       if(current_state==RETURNING) current_state=IDLE;
       else current_state = ARRIVED;
    }
    };
    nav_client_->async_send_goal(goal, send_goal_options);
    if(current_state==MOVING)RCLCPP_INFO(this->get_logger(),"%s구역(%.1f , %.1f)으로 이동중",zone_name.c_str(),x,y); 
    else if(current_state==RETURNING)RCLCPP_INFO(this->get_logger(),"HOME(%.1f , %.1f)으로 복귀중",x,y);
  } 

}
);

timer_ = this->create_wall_timer(std::chrono::milliseconds(100),
[this]()
{
 static int count=0;
 
switch(current_state)
{ 
 case IDLE:
 {  
  if((count++ % 50)==0)RCLCPP_INFO(this->get_logger(),"[IDLE]명령 대기중");

 break;
 }
 
 case MOVING:
 {
 if((count++ % 5)==0)RCLCPP_INFO(this->get_logger(),"[MOVING]%s구역으로 이동중",zone_name.c_str());
 
 break;
 
 }

 case ARRIVED:
 {
 
 if((count++ % 5)==0)RCLCPP_INFO(this->get_logger(),"[ARRIVED]%s구역에 도착. 사용자 탐색중",zone_name.c_str());
 
 break;
 
 }

 case RETURNING:
 {
 if((count++ % 5)==0)RCLCPP_INFO(this->get_logger(),"[RETURNING]%s구역 탐색 마치고 복귀중",zone_name.c_str());
 
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