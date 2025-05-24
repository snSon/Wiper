// Module Name : obstacle_stop_planning.cpp
// 
// Data : 2025-05-17 
// Name : LeeJunYeong

// #include "processing/object_detection"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <iomanip>

const float GRAVITY = 9.8;     // 중력 가속도

// 여기는 변경해서 사용하면 됨
struct DetectedObject {
    float x_center;     // 인식 객체 박스 가운데 x 
    float y_center;     // 인식 객체 박스 가운데 y
    float bbox_width;   // 바운딩 박스 폭 (픽셀)
    float bbox_height;  // 바운딩 박스 높이 (픽셀)
    // float distance_m;   // 장애물까지 거리 (m)
    // float velocity_mps; // 상대 속도 (m/s)
};

// 거리 임의 계산
double estimate_distance_from_bbox(const DetectedObject& obj, const std::string& label) {
    // 객체의 실제 높이 (meters)
    std::unordered_map<std::string, double> known_heights = {
        {"person", 1.7},
        {"car", 1.5},
        {"truck", 2.5}
    };

    // 카메라 내부 파라미터: 초점 거리 (픽셀 단위로 대략 지정)
    const double focal_length_px = 300.0;
    // 라벨 없을 경우 기본 높이
    const double default_height_m = 1.5;

    double object_height_m = default_height_m;
    auto it = known_heights.find(label);
    if (it != known_heights.end()) {
        object_height_m = it->second;
    } else {
        std::cerr << "기본 높이 " << default_height_m << "m 사용\n";
    }

    double bbox_height_px = obj.bbox_height;

    if (bbox_height_px <= 0) {
        std::cerr << "잘못된 bbox height" << std::endl;
        return -1.0;
    }

    double distance_m = object_height_m * focal_length_px / bbox_height_px;
    return distance_m;
}
// 파라미터 (Autoware에서 가져옴)
const float MIN_DIST_TO_OBJECT = 1.0;     // 최소 정지 거리
const float MARGIN_TIME_SEC    = 2.0;     // 충돌 임박 판단 기준 시간

// 반응 시간 계산 공식
double compute_total_braking_distance(double velocity_mps, double reaction_time_sec, double friction) {
    double braking_distance = (velocity_mps * velocity_mps) / (2 * friction * GRAVITY);
    double reaction_distance = velocity_mps * reaction_time_sec;
    return braking_distance + reaction_distance;
}

void simulate_wet_braking(double initial_velocity_kmph, double distance_to_object) {
    const double initial_velocity = initial_velocity_kmph * 1000 / 3600;
    const double target_stop_distance = 1.0;
    const double total_braking_distance = distance_to_object - target_stop_distance;
    const double dt = 0.05;

    double current_velocity = initial_velocity;
    double total_distance = 0.0;
    double time_elapsed = 0.0;

    std::cout << "\n[선형 감속 시작 - 목표: " << target_stop_distance << "m 앞 정지]\n";

    while (total_distance < total_braking_distance) {
        double remaining_distance = total_braking_distance - total_distance;
        if (remaining_distance < 0.001) break;

        // 선형 감속: 거리 비율 기반 속도 감소
        current_velocity = initial_velocity * (remaining_distance / total_braking_distance);
        double step_distance = current_velocity * dt;
        total_distance += step_distance;

        std::cout << std::fixed << std::setprecision(2)
                  << "[t=" << time_elapsed << "s] 속도: " << current_velocity << " m/s  "
                  << "이동 거리: " << step_distance << " m  "
                  << "누적 거리: " << total_distance << " m\n";

        time_elapsed += dt;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "\n 정지 완료!" << target_stop_distance << "m에서 멈춤\n";
}

// wet 환경 제동 거리
double calculate_braking_distance(const DetectedObject& obj, double ego_velocity, double reaction_time_sec) {
    double friction_Coe = 0.8; // 마찰 계수 일반 아스팔트
    double friction_coe_wet = 0.35;   // 마찰 계수 젖은 아스팔트
    double vehicle_distance = ego_velocity * 1000 / 3600; // 차량 1초 동안 가는 거리 공식
    double wet_braking_distance = pow(vehicle_distance, 2) / (2 * friction_coe_wet * GRAVITY);
    double reaction_distance = vehicle_distance * reaction_time_sec; // 반응거리
    wet_braking_distance = wet_braking_distance + reaction_distance;
    std::string label = "test";
    double distance_m = estimate_distance_from_bbox(obj, label);

    std::cout << "[판단] 객체 거리: " << distance_m << "m / 제동 거리: " << wet_braking_distance << "m\n";

    if (distance_m <= wet_braking_distance) {
        std::cout << "긴급 정지!"<< "\n";
    } else if (distance_m > wet_braking_distance) {
        std::cout << "감속 시작" << "\n";
        simulate_wet_braking(ego_velocity, distance_m);
    } else {
        std::cout << "정상 주행"<< "\n";
    }
    return 0;
}

int main() {
    double current_speed = 50.0;
    double target_speed = 0.0;
    // 예시: 객체 인식 결과 입력
    std::vector<DetectedObject> objects = {
        {5.0f, -2.0f, 0.5f, 0.5f},
    };

    auto start = std::chrono::steady_clock::now();
    compute_total_braking_distance(current_speed * 1000 / 3600, 0, 0.35);  // 임시
    auto end = std::chrono::steady_clock::now();

    double reaction_time_sec = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
    std::cout << std::fixed << std::setprecision(6) << "반응시간: " << reaction_time_sec << "\n";
    // 다시 판단 (정확한 반응 시간 반영)
    calculate_braking_distance(objects[0], current_speed, reaction_time_sec);

    return 0;
}
