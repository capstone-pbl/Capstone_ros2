// 라이다에서 측정한 데이터(pi)->laptop



#include "rclcpp/rclcpp.hpp"
#include "my_robot_controller/PI_TO_LAPTOP/laser_scan.hpp"









int main(int argc,char ** argv)
{
  rclcpp::init(argc,argv);
  auto node=std::make_shared<Laser_Scan>();
  rclcpp::spin(node);
  rclcpp::shutdown();


    return 0;
}