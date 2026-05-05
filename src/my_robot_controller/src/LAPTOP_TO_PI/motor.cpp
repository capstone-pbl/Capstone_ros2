#include "rclcpp/rclcpp.hpp"
#include "my_robot_controller/LAPTOP_TO_PI/motor.hpp"
#include <cmath>
#include <fcntl.h>    // 포트 제어 (open)
#include <termios.h>  // 시리얼 통신 설정 (Baudrate 등)
#include <unistd.h>   // write, close
#include <cstdio>     // sprintf

#define L 0.288
#define circumference (M_PI * 0.065)

Motor::Motor() : Node("Motor"), serial_fd(-1)
{
    // 1. 시리얼 포트 초기화 (열기)
    init_serial();

    // 2. 실제 속도 발행 통로 (나중에 피드백 받을 때 사용)
    pub_actual_vel = this->create_publisher<geometry_msgs::msg::Twist>("/actual_vel", 10);

    // 3. /cmd_vel 구독 및 STM32 전송
    sub_cmd = this->create_subscription<geometry_msgs::msg::Twist>("/cmd_vel", 10,
    [this](geometry_msgs::msg::Twist::SharedPtr msg)
    {
        double v = msg->linear.x;
        double w = msg->angular.z;

        // Kinematics 계산 (좌/우 바퀴 속도)
        double v_left = v - w * (L / 2.0);
        double v_right = v + w * (L / 2.0);

        // RPM 변환 (바퀴 지름 0.065m 기준)
      
        double L_RPM = (v_left * 60.0) / circumference;
        double R_RPM = (v_right * 60.0) / circumference;

        RCLCPP_INFO(this->get_logger(), "좌 RPM: %.2f, 우 RPM: %.2f", L_RPM, R_RPM);

        // --- [작업 1: 문자열 패킷 변환 및 UART 전송] ---
        if (serial_fd != -1) {
            char tx_buffer[64];
           
            int len = sprintf(tx_buffer, "M%.2f,%.2f\n", L_RPM, R_RPM);
            //tx_buffer에 "M%.2f,%.2f\n"형태로 저장 
            // 실제로 UART 선으로 데이터 발사
            write(serial_fd, tx_buffer, len);
            RCLCPP_INFO(this->get_logger(), "STM32 전송: %s", tx_buffer);
        }
    }); 

   timer_= this->create_wall_timer(std::chrono::milliseconds(100),
   [this]()
   {
    double L_RPM, R_RPM;
    char rx_buffer[64];
     RCLCPP_INFO(this->get_logger(),"타이머 호출됨");
     int n = read(serial_fd,rx_buffer,sizeof(rx_buffer) - 1);   //read(serial_fd, 버퍼, 크기); null문자 자리 확보
    // 최대 rx_buffer의 크기 - 1 만큼 데이터를 한번에 읽어옴      
     rx_buffer[n] = '\0';
     if(n>0)
     {
        RCLCPP_INFO(this->get_logger(),"rx 데이터:%s",rx_buffer);     
        sscanf(rx_buffer,"F%lf,%lf\n",&L_RPM,&R_RPM);
        RCLCPP_INFO(this->get_logger(),"L_RPM:%.2f, R_RPM:%.2f",L_RPM,R_RPM);
       
         double v_left = (L_RPM*circumference) /60.0;
         double v_right = (R_RPM*circumference) /60.0;
         double v= (v_left+v_right)/2.0;
         double w= (v_right-v_left)/L;
      
         
         auto msg=geometry_msgs::msg::Twist();
         msg.linear.x = v;
         msg.angular.z = w;
         pub_actual_vel->publish(msg);
    }
    
    
    
    }

    );


}

// 시리얼 포트를 설정하고 여는 함수
void Motor::init_serial() 
{    // 우분투 시리얼 포트 열기
    serial_fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (serial_fd == -1) {
        RCLCPP_ERROR(this->get_logger(), "시리얼 포트를 열 수 없습니다! /dev/serial0를 확인하세요.");
        return;
}

    struct termios options;
    tcgetattr(serial_fd, &options);

    // 통신 속도 115200 Baud
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    // 8N1 설정 (데이터 8비트, 패리티 없음, 정지 비트 1개)
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;
    // tcsetattr 전에 추가
    options.c_iflag &= ~(IXON | IXOFF | IXANY);  // 소프트웨어 흐름 제어 끔
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // raw 모드
    options.c_oflag &= ~OPOST;  // 출력 처리 끔
    tcsetattr(serial_fd, TCSANOW, &options);
    RCLCPP_INFO(this->get_logger(), "시리얼 포트 설정 완료 (115200)");
}

// 노드 종료 시 포트 닫기
Motor::~Motor() {
    if (serial_fd != -1) {
        close(serial_fd);
    }
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Motor>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}