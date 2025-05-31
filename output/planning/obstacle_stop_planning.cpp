// Module Name : obstacle_stop_planning.cpp
// 
// Data : 2025-05-31
// Name : LeeJunYeong

// #include "output/planning/"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <iomanip>
#include <deque>
#include <queue>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
// 거리 데이터 헤더파일 프레임 307개 기준
#include "frame_distances.hpp"

const float GRAVITY = 9.8;     // 중력 가속도
const int MAX_OBJECTS = 20; // 인식 객체 최대수
const int FRAME_COUNT = 932; // 영상의 프레임 수
const char* shm_name = "/detection_buffer";
// 실제 데이터를 기준으로 변경
const float ref_x = 0.37f;  // 기준 차의 bbox x_center
const float ref_y = 0.45f;  // 기준 차의 bbox y_center
const float tolerance = 0.03f; // 기준 허용 오차

struct DetectedObject {
    float x_center;     // 인식 객체 박스 가운데 x 
    float y_center;     // 인식 객체 박스 가운데 y
    float bbox_width;   // 바운딩 박스 폭 (픽셀)
    float bbox_height;  // 바운딩 박스 높이 (픽셀)
    double manual_distance = -1.0; // 거리값
};

// 대상 객체인지 판별
bool is_target_object(const DetectedObject& obj) {
    return std::abs(obj.x_center - ref_x) < tolerance &&
           std::abs(obj.y_center - ref_y) < tolerance;
}

// 공유 메모리에서 객체 목록 읽기
std::vector<DetectedObject> read_shared_objects(int frame_id) {
    size_t OBJECT_SIZE = sizeof(DetectedObject);
    size_t FRAME_SIZE = MAX_OBJECTS * OBJECT_SIZE;

    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1){
        std::cerr << "공유 메모리 열기 실패" << std::endl;
        exit(1);
    }

    void* ptr = mmap(0, FRAME_COUNT * FRAME_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "mmap 실패" << std::endl;
        exit(1);
    }


    DetectedObject* objects_all = static_cast<DetectedObject*>(ptr);
    DetectedObject* frame_ptr = objects_all + frame_id * MAX_OBJECTS;

    std::vector<DetectedObject> result;
    for(int i = 0; i < MAX_OBJECTS; ++i) {
        if (frame_ptr[i].bbox_height <= 0.0f) break;
        result.push_back(frame_ptr[i]);
    }

    munmap(ptr, FRAME_COUNT * FRAME_SIZE);
    close(shm_fd);
    return result;
}

// wet 환경 제동 거리
double calculate_braking_distance(const DetectedObject& obj, double ego_velocity, int weather) {
    double fc = 0.8f; // 디폴트 마찰계수 0.8
    if (weather == 1) {
        fc = 0.4f;    // 비옴 마찰계수 0.4
    }
    double vehicle_distance = ego_velocity * 1000 / 3600; // 차량 1초 동안 가는 거리 공식
    double braking_distance = pow(vehicle_distance, 2) / (2 * fc * GRAVITY);
    double reaction_distance = vehicle_distance * 0.1; // 반응 거리 시스템 안전성을 위해 0.1
    braking_distance = braking_distance + reaction_distance;
    return braking_distance;
}
// 주행 추종 함수
void match_object_speed(double object_speed_mps) {
    std::cout << "객체 속도에 맞춰 주행 시작... 대상 속도: " << object_speed_mps << " m/s\n";
    const double dt = 0.1;
    double total_distance = 0.0;
    double time_elapsed = 0.0;

    for (int i = 0; i < 50; ++i) {
        double step = object_speed_mps * dt;
        total_distance += step;
        time_elapsed += dt;

        std::cout << "[t=" << time_elapsed << "s] 속도: " << object_speed_mps << " m/s, 이동 거리: " << step << " m\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
// 객체 속도 추정 -> 제어 신호 생성 (함수명 바꾸자)
void control_signal(const DetectedObject& obj, double ego_velocity_kmph, int weather) {
    // 인식된 객체 속도 0으로 함
    double relative_speed = 0.0f;

    // 안전거리
    double safe_distance = calculate_braking_distance(obj, ego_velocity_kmph, weather);
    // 객체와의 거리
    double distance_m = obj.manual_distance;
    std::cout << std::fixed << std::setprecision(2)
              << "[데이터] 거리: " << distance_m << " m / 제동 안전 거리: " << safe_distance << " m\n";

    // 거리 + 속도 기반
    if (distance_m <= (safe_distance + 5.0) && std::abs(relative_speed) < 1.0) {
        std::cout << "[판단] 전방에 멈춘 객체 인식 → 감속 후 정지 시작\n";
        // simulate_braking(ego_velocity_kmph, distance_m);
    }
    else {
        std::cout << "[판단] 주행 중인 객체 → 속도 맞춰 추종\n";
        match_object_speed(relative_speed);
    }
}


int main() {
    for (int frame = 0; frame < FRAME_COUNT; ++frame) {
        std::cout << "\n========================\n";
        std::cout << "[프레임 " << frame << "]\n";
        auto objects = read_shared_objects(frame);
        if(objects.empty()) {
            std::cout << "[정보] 유효한 객체 없음.\n";
            continue;
        }
        for (auto& obj : objects) {
            // 대상 객체일 경우 거리 수동 주입
            if (is_target_object(obj)) {
                auto it = frame_to_distance.find(frame);
                if (it != frame_to_distance.end()) {
                    obj.manual_distance = it->second;
                }
            }
            // 일반 = 0, 비옴 = 1
            int weather = 1;
            control_signal(obj, 60.0f, weather);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    return 0;
}

