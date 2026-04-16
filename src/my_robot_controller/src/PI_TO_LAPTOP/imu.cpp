#include "rclcpp/rclcpp.hpp"
#include "my_robot_controller/imu.hpp"














int main(int argc,char ** argv)
{

rclcpp::init(argc,argv);
auto imu=std::make_shared<Imu>();
rclcpp::spin(imu);
rclcpp::shutdown();


return 0;


}