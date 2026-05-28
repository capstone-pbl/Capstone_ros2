#include "rclcpp/rclcpp.hpp"
#include "my_robot_controller/LAPTOP_TO_PI/motor.hpp"
#include <cmath>
#include <fcntl.h>    // 포트 제어 (open)
#include <termios.h>  // 시리얼 통신 설정 (Baudrate 등)
#include <unistd.h>   // write, close
#include <cstdio>     // sprintf

#define L 0.288
#define circumference (M_PI * 0.067)
int dbg_fd=-1;
volatile unsigned long tim10_cnt;
Motor::Motor() : Node("Motor"), serial_fd(-1),last_write_time(this->now())
{
    // 1. 시리얼 포트 초기화 (열기)
    init_serial();

    // 2. 실제 속도 발행 통로 (나중에 피드백 받을 때 사용)

    pub_actual_vel = this->create_publisher<geometry_msgs::msg::Twist>("/actual_vel", 10);

    // 3. /cmd_vel 구독 및 STM32 전송
    sub_cmd = this->create_subscription<geometry_msgs::msg::Twist>("/cmd_vel", 10,
    [this](geometry_msgs::msg::Twist::SharedPtr msg)
    {
        auto now = this->now();
        if ((now - last_write_time).seconds() < 0.05) return;
        last_write_time = now;
       
        double v = msg->linear.x;
        double w = msg->angular.z;
        if(v<0)w=-w;
        // Kinematics 계산 (좌/우 바퀴 속도)
        double v_left = v - w * (L / 2.0);
        double v_right = v + w * (L / 2.0);

        // RPM 변환 (바퀴 지름 0.065m 기준)
      
        double L_RPM = (v_left * 60.0) / circumference;
        double R_RPM = (v_right * 60.0) / circumference;
        
         // dead zone 처리
        const double MIN_RPM = 65.0; // 테스트로 찾은 최소 동작 RPM
        if (v != 0.0 || w != 0.0)
        {
        if (L_RPM > 0 && L_RPM < MIN_RPM) L_RPM = MIN_RPM;
        if (L_RPM < 0 && L_RPM > -MIN_RPM) L_RPM = -MIN_RPM;
        if (R_RPM > 0 && R_RPM < MIN_RPM) R_RPM = MIN_RPM;
        if (R_RPM < 0 && R_RPM > -MIN_RPM) R_RPM = -MIN_RPM;
        }
        RCLCPP_INFO(this->get_logger(), "좌 RPM: %.2f, 우 RPM: %.2f,", L_RPM, R_RPM);

        // 문자열 패킷 변환 및 UART 전송
        if (serial_fd != -1) {
            char tx_buffer[64];
           
            int len = sprintf(tx_buffer, "M%.2f,%.2f\n", L_RPM, R_RPM);
            //tx_buffer에 "M%.2f,%.2f\n"형태로 저장 
            // 실제로 UART 선으로 데이터 전송
            RCLCPP_INFO(this->get_logger(),"write전");
           
           int ret = write(serial_fd, tx_buffer, len);
           if(ret < 0) {
           char buf[32];
           int n = snprintf(buf, sizeof(buf), "WRITE_FAIL errno=%d\n", errno);
           write(STDERR_FILENO, buf, n);
}
            RCLCPP_INFO(this->get_logger(),"write후");
            RCLCPP_INFO(this->get_logger(), "STM32 전송: %s", tx_buffer);
        
        }
    }); 

   timer_= this->create_wall_timer(std::chrono::milliseconds(100),
   [this]()
   {
    double L_RPM, R_RPM;
    char rx_buffer[64];
     RCLCPP_INFO(this->get_logger(),"타이머 호출됨");
     write(dbg_fd, "1\n", 2);
     int flags = fcntl(serial_fd, F_GETFL);
     write(dbg_fd, "2\n", 2);
     RCLCPP_INFO(this->get_logger(), "serial_fd: %d, O_NONBLOCK: %d", serial_fd, (flags & O_NONBLOCK));
     RCLCPP_INFO(this->get_logger(),"read전");
     write(dbg_fd, "3\n", 2);
     int n = read(serial_fd,rx_buffer,sizeof(rx_buffer) - 1);   //read(serial_fd, 버퍼, 크기); null문자 자리 확보
     write(dbg_fd, "4\n", 2);
     RCLCPP_INFO(this->get_logger(),"read후, n=%d, error=%d",n,errno);
    // 최대 rx_buffer의 크기 - 1 만큼 데이터를 한번에 읽어옴      
    if(n>0)
     {
        write(dbg_fd, "5\n", 2);
        rx_buffer[n] = '\0';
        RCLCPP_INFO(this->get_logger(),"rx 데이터:%s",rx_buffer);     
        sscanf(rx_buffer,"F%lf,%lf,%lu\n",&L_RPM,&R_RPM,&tim10_cnt);
         write(dbg_fd, "6\n", 2);
       RCLCPP_INFO(this->get_logger(),"L_RPM:%.2f, R_RPM:%.2f, TIM10:%lu", L_RPM, R_RPM, tim10_cnt);
         double v_left = (L_RPM*circumference) /60.0;
         double v_right = (R_RPM*circumference) /60.0;
         double v= (v_left+v_right)/2.0;
         double w= (v_right-v_left)/L;
      
         
         auto msg=geometry_msgs::msg::Twist();
         msg.linear.x = v;
         msg.angular.z = w;
        RCLCPP_INFO(this->get_logger(),"pub_actual_vel전");
         pub_actual_vel->publish(msg);
         write(dbg_fd, "7\n", 2);
      
    }
       write(dbg_fd, "8\n", 2);
    
    
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
    fcntl(serial_fd, F_SETFL, O_NONBLOCK);
    dbg_fd = open("/home/turtle/dbg.txt", O_WRONLY|O_CREAT|O_APPEND, 0644);
    RCLCPP_INFO(this->get_logger(), "시리얼 포트 설정 완료 (115200)");
}

// 노드 종료 시 포트 닫기
Motor::~Motor() {
    if (serial_fd !=-1) {
        close(serial_fd);
    }
}

int main(int argc, char **argv)
{
   rclcpp::init(argc, argv);
  signal(SIGINT, [](int){ write(dbg_fd, "SIGINT\n", 7); });
  signal(SIGSEGV, [](int){ write(dbg_fd, "SIGSEGV\n", 8); });
  signal(SIGHUP, [](int){ write(dbg_fd, "SIGHUP\n", 7); });
   
    auto node = std::make_shared<Motor>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}