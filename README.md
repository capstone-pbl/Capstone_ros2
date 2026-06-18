# 🤖 Capstone ROS2 — 자율주행 스마트 쓰레기통 ROS2 패키지

> ROS2 Humble 기반 자율주행 로봇 제어 노드 | 캡스톤 디자인 동상 수상

## 프로젝트 개요

Blynk 앱에서 구역(A/B/C)을 호출하면 RPLiDAR C1으로 생성한 맵 위에서 Nav2가 자율주행으로 해당 구역까지 이동하고, 도착 후 STM32가 뚜껑을 개폐한 뒤 홈으로 복귀하는 **스마트 쓰레기통 로봇**의 ROS2 노드 패키지다.

- 동적 장애물 회피 포함 시연 성공
- 캡스톤 디자인 **동상 수상**

## 시스템 전체 구조

```
[Blynk 앱]
    ↓ MQTT → /call_zone
[Brain 노드] ── nav2_msgs/NavigateToPose ──→ [Nav2 Stack]
                                                  ↑
[Odom 노드] → /odom ──────────────────────────────┤
[LiDAR] → /scan ──────────────────────────────────┘

[Nav2] → /cmd_vel → [Motor 노드]
                         ↓ UART (115200bps)
                    [STM32F411]
                         ↑ "F{L_rpm},{R_rpm},{tick}\n"
                    [Motor 노드] → /actual_vel → [Odom 노드]
```

## 노드 구성

### 📁 LAPTOP_TO_PI (랩탑 → RPi 방향 제어)

#### `Brain` 노드
상태머신으로 전체 미션을 관리한다.

| 상태 | 설명 |
|------|------|
| `IDLE` | 명령 대기 |
| `MOVING` | 목표 구역으로 이동 중 |
| `ARRIVED` | 목표 구역 도착 |
| `RETURNING` | 홈으로 복귀 중 |

**구역 좌표 (맵 기준)**

| 구역 | X | Y |
|------|---|---|
| home | 0.10 | 0.005 |
| A | 2.23 | -2.09 |
| B | -7.28 | 7.96 |
| C | -4.45 | 7.08 |

- `/call_zone` 토픽 수신 → Nav2 `navigate_to_pose` Action 호출
- 도착 결과 콜백으로 상태 전이
- `/brain_state` 토픽으로 현재 상태 100ms 주기 퍼블리시

#### `Motor` 노드
Nav2의 `/cmd_vel`을 받아 역기구학으로 변환 후 STM32에 UART 전송하고, 피드백을 수신해 오도메트리에 전달한다.

**역기구학 (Differential Drive)**
```
v_left  = v - ω × (L/2)    // L = 바퀴간격 0.288m
v_right = v + ω × (L/2)

L_RPM = v_left  × 60 / (π × 0.067)
R_RPM = v_right × 60 / (π × 0.067)
```

**UART 프로토콜**
```
RPi → STM32:  "M{L_rpm},{R_rpm}\n"       // 모터 RPM 명령
STM32 → RPi:  "F{L_rpm},{R_rpm},{tick}\n" // 엔코더 피드백
```

- Dead zone 처리: 최소 동작 RPM(65 RPM) 이하 입력 시 클램핑
- `/dev/serial0` (115200bps, 8N1, raw 모드, non-blocking)
- 50ms throttle로 cmd_vel 처리율 제한

---

### 📁 PI_TO_LAPTOP (RPi → 랩탑 방향 데이터)

#### `Odom` 노드
Motor 노드의 `/actual_vel` 피드백을 받아 데드레코닝으로 로봇 위치를 추정하고 Nav2에 제공한다.

```
θ += ω × dt
x += v × cos(θ) × dt
y += v × sin(θ) × dt
```

- `/odom` 토픽 퍼블리시 (`nav_msgs/Odometry`)
- `odom` → `base_footprint` TF 브로드캐스트 (tf2)
- 노이즈 필터: |v| < 0.01, |ω| < 0.05 시 0으로 처리

---

## 토픽 구조

| 토픽 | 타입 | 방향 | 설명 |
|------|------|------|------|
| `/call_zone` | `std_msgs/String` | 수신 | Blynk 구역 호출 ("A"/"B"/"C"/"home") |
| `/brain_state` | `std_msgs/String` | 발행 | 현재 상태머신 상태 |
| `/cmd_vel` | `geometry_msgs/Twist` | 수신 | Nav2 속도 명령 |
| `/actual_vel` | `geometry_msgs/Twist` | 발행 | STM32 엔코더 피드백 기반 실제 속도 |
| `/odom` | `nav_msgs/Odometry` | 발행 | 데드레코닝 위치 추정 |
| `/scan` | `sensor_msgs/LaserScan` | 수신 | RPLiDAR C1 스캔 데이터 |

## Launch 파일

```bash
# SLAM 맵 생성
ros2 launch my_robot_controller slam_launch.py

# Nav2 자율주행 (맵 지정)
ros2 launch my_robot_controller nav2_launch.py map:=$HOME/maps/map.yaml

# RPLiDAR C1 드라이버
ros2 launch my_robot_controller rplidar_c1_launch.py
```

## 소프트웨어 스택

| 항목 | 버전/내용 |
|------|----------|
| OS | Ubuntu 22.04 (RPi 4B) |
| ROS | ROS2 Humble |
| 자율주행 | Nav2 (MPPI Planner + RotationShimController) |
| SLAM | SLAM Toolbox (localization 모드) |
| DDS | CycloneDDS |
| 크로스머신 통신 | Tailscale VPN (WSL ↔ RPi) |
| IoT | Blynk IoT (MQTT) |

## 빌드 방법

```bash
cd ~/ros2_ws
colcon build --packages-select my_robot_controller
source install/setup.bash
```

## 담당 역할

- **Brain 노드**: 상태머신 설계 및 Nav2 Action 클라이언트 구현
- **Motor 노드**: 역기구학 계산, UART 시리얼 통신, STM32 프로토콜 연동
- **Odom 노드**: 데드레코닝 위치 추정, TF 브로드캐스트
- **CycloneDDS 설정**: Tailscale 기반 WSL ↔ RPi 크로스머신 ROS2 통신
- **Blynk MQTT 연동**: 구역 호출 → `/call_zone` 토픽 트리거
