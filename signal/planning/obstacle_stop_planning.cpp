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
#include <fstream>
// 거리 데이터 헤더파일
#include "frame_distances.hpp"
// 사용할 객체 인식 로그 위치 
const std::string LOG_DIR = "/detect_log";

const float GRAVITY = 9.8;     // 중력 가속도
const int MAX_OBJECTS = 20; // 인식 객체 최대수
const int FRAME_COUNT = 424; // 영상의 프레임 수

// 실제 데이터를 기준으로 변경
const float ref_x = 970.0f;  // 기준 차의 bbox x_center
const float ref_y = 720.0f;  // 기준 차의 bbox y_center
const float tolerance = 20.0f; // 기준 허용 오차

struct DetectedObject {
    float x_center;     // 인식 객체 박스 가운데 x 
    float y_center;     // 인식 객체 박스 가운데 y
    float bbox_width;   // 바운딩 박스 폭 (픽셀)
    float bbox_height;  // 바운딩 박스 높이 (픽셀)
    float confidence;
    double manual_distance = 100.0;  // 초기 거리값
};
// 로그 저장 함수
void save_log(int frame, const DetectedObject& obj, const std::string& decision) {
    std::ofstream log_file("/control_log/control_log.txt", std::ios::app); // append 모드
    if (!log_file.is_open()) {
        std::cerr << "[오류] 로그 파일 열기 실패\n";
        return;
    }
    if (obj.manual_distance != 100.0) {
        log_file << "[프레임 " << frame << "] "
            << "거리: " << std::fixed << std::setprecision(2) << obj.manual_distance << " m, "
            << "x_center: " << obj.x_center << ", "
            << "y_center: " << obj.y_center << ", "
            << "판단: " << decision << "\n";
    }
    log_file.flush(); 
    log_file.close();
}
// 대상 객체인지 판별
bool is_target_object(const DetectedObject& obj) {
    return std::abs(obj.x_center - ref_x) < tolerance &&
           std::abs(obj.y_center - ref_y) < tolerance;
}

// 객체 인식 로그 파일 읽기
std::vector<DetectedObject> load_objects_from_log(int frame_id) {
    std::vector<DetectedObject> objects;
    std::ostringstream filepath;
    filepath << LOG_DIR << "/frame_" << std::setw(3) << std::setfill('0') << frame_id << ".txt";

    std::ifstream file(filepath.str());
    if (!file.is_open()) {
        std::cerr << "[경고] 로그 파일 열기 실패: " << filepath.str() << "\n";
        return objects;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        DetectedObject obj;
        if (!(ss >> obj.class_id >> obj.x_center >> obj.y_center >>
              obj.bbox_width >> obj.bbox_height >> obj.confidence)) {
            std::cerr << "[경고] 파싱 실패한 줄: " << line << "\n";
            continue;
        }
        objects.push_back(obj);
    }

    return objects;
}
// wet 환경 제동 거리
double calculate_braking_distance(const DetectedObject& obj, double ego_velocity_kmph, int weather) {
    double fc = 0.8f; // 디폴트 마찰계수 0.8
    if (weather == 1) {
        fc = 0.4f;    // 비옴 마찰계수 0.4
    }
    double vehicle_distance = ego_velocity_kmph * 1000 / 3600; // 차량 1초 동안 가는 거리 공식
    double braking_distance = pow(vehicle_distance, 2) / (2 * fc * GRAVITY);
    double reaction_distance = vehicle_distance * 0.1; // 반응 거리 시스템 안전성을 위해 0.1
    braking_distance = braking_distance + reaction_distance;
    return braking_distance;
}
// 주행 추종 함수
void match_object_speed(const DetectedObject& obj, double ego_velocity_kmph, double object_speed_mps, int weather) {
    double distance_m = obj.manual_distance;
    double safe_distance = calculate_braking_distance(obj, ego_velocity_kmph, ,weather);

    if (distance_m + 5.0 <= safe_distance) {
        std::cout << "객체 속도에 맞춰 주행 시작... 대상 속도: " << object_speed_mps << " m/s\n";
    }
}
// 객체 속도 추정 -> 제어 신호 생성
void control_signal(const DetectedObject& obj, double ego_velocity_kmph, int weather, int frame) {
    // 인식된 객체 속도 0으로 함
    double relative_speed = 0.0f;

    // 안전거리
    double safe_distance = calculate_braking_distance(obj, ego_velocity_kmph, weather);
    // 객체와의 거리
    double distance_m = obj.manual_distance;
    if (distance_m != 100.0f) {
        std::cout << std::fixed << std::setprecision(2)
            << "[데이터] 거리: " << distance_m << " m / 제동 안전 거리: " << safe_distance << " m\n";
    }
    std::string decision;
    // 거리 + 속도 기반
    if (distance_m <= (safe_distance + 5.0) && std::abs(relative_speed) < 1.0) {
        decision = "감속 후 정지";
        std::cout << "[판단] 전방에 멈춘 객체 인식 → 감속 후 정지 시작\n";
    }
    else {
        decision = "속도 맞춰 추종";
        std::cout << "[판단] 주행 중인 객체 → 속도 맞춰 추종\n";
        match_object_speed(obj, ego_velocity_kmph, relative_speed, weather);
    }
}


int main() {
    for (int frame = 0; frame < FRAME_COUNT; ++frame) {
        std::cout << "\n========================\n";
        std::cout << "[프레임 " << frame << "]\n";
        auto objects = load_objects_from_log(frame);
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
            control_signal(obj, 60.0f, weather, frame);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    return 0;
}

