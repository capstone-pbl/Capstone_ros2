#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/Odometry.hpp"
class SimpleController : public rclcpp::Node
//rclcpp:네임스페이스 Node:클래스
{//Ros2의 모든 기능을 가진 부모를 상속받음
public:

    SimpleController() : Node("Simple_node") //부모 생성자에 이름을 던져줌
    {
       sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>
    ("/scan",10,
    [this](const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
         RCLCPP_INFO(this->get_logger(),"거리: %f m",msg->ranges[0]);
           this->current_distance=msg->ranges[0];
         if(this->current_distance<0.5)
         {
          RCLCPP_WARN(this->get_logger(),"거리 너무 가까움");
         }
        
   }  
    );
       
       pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel",10);

       
       timer_ = this->create_wall_timer(std::chrono::milliseconds(100),
       [this]()
       {
        auto msg = geometry_msgs::msg::Twist();
        msg.linear.x = 0.5;   // 전진
        msg.angular.z = 0.0;   // 회전속도 0
        
        pub_->publish(msg);
       });
     

 
// sub_   -> 데이터 올 때 호출됨 -> msg 받아야 함 -> (msg)
// timer_ -> 시간 되면 호출됨   -> 받을 게 없음 -> ()
// pub_   -> 콜백 없음          -> 내가 직접 publish 호출             
    } 

  


private:
   rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr sub_;
   rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_;
   rclcpp::TimerBase::SharedPtr timer_;
   double current_distance;
};



//ROS2: spin이 while(1) 역할, 로직은 클래스 안 콜백에서 실행


int main(int argc,char **argv) //터미널 내에서 코드 동적으로 실행
{
 rclcpp::init(argc,argv); //Ros2 통신시스템 켜기, 터미널 내에서 코드 동적으로 실행,
 //DDS(통신 관리자)를 상속받아서 띄움
 auto node = std::make_shared<SimpleController>(); //shared_ptr로 SimpleController 클래스의 
 // 객체 노드생성
 rclcpp::spin(node); //노드 가동(이벤트 대기 루프), 무한대기실
 rclcpp::shutdown(); //Ros2 종료
 
 
 return 0;

}


// 1. 클래스 만들고 Node 상속
// 2. 생성자에서 create_subscription / create_publisher / create_wall_timer
// 3. private에 SharedPtr로 저장
// 4. main은 init -> make_shared -> spin -> shutdown