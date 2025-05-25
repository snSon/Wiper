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
#include <deque>
#include <queue>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

const float GRAVITY = 9.8;     // 중력 가속도
const int MAX_OBJECTS = 20; // 인식 객체 최대수
const int FRAME_COUNT = 932; // 영상의 프레임 수
const char* shm_name = "/detection_buffer";

// 여기는 변경해서 사용하면 됨
struct DetectedObject {
    float x_center;     // 인식 객체 박스 가운데 x 
    float y_center;     // 인식 객체 박스 가운데 y
    float bbox_width;   // 바운딩 박스 폭 (픽셀)
    float bbox_height;  // 바운딩 박스 높이 (픽셀)
};

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
// const float MARGIN_TIME_SEC    = 2.0;     // 충돌 임박 판단 기준 시간

// 객체 속도 유추
struct TrackedObject {
    std::string label;
    std::deque<DetectedObject> history;
    std::deque<double> distances;
    std::deque<std::chrono::steady_clock::time_point> timestamps;

    const size_t max_history = 5;

    void add_observation(const DetectedObject& obj) {
        double distance = estimate_distance_from_bbox(obj, label);
        auto now = std::chrono::steady_clock::now();

        if (distance > 0) {
            history.push_back(obj);
            distances.push_back(distance);
            timestamps.push_back(now);

            if (history.size() > max_history) {
                history.pop_front();
                distances.pop_front();
                timestamps.pop_front();
            }
        }
    }

    // 상대 속도 계산 (m/s)
    double estimate_relative_speed() const {
        if (distances.size() < 2) return 0.0;

        size_t i = 0;
        size_t j = distances.size() - 1;

        double d1 = distances[i];
        double d2 = distances[j];

        double dt_sec = std::chrono::duration_cast<std::chrono::milliseconds>(
                            timestamps[j] - timestamps[i]).count() / 1000.0;

        if (dt_sec <= 0.0) return 0.0;

        return (d1 - d2) / dt_sec;  // 양수면 가까워짐, 음수면 멀어짐
    }
};

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
// 객체 속도 추정
void update_and_decide_with_distance_and_speed(TrackedObject& tracked, const DetectedObject& current_obj, double ego_velocity_kmph) {
    tracked.add_observation(current_obj);
    double relative_speed = tracked.estimate_relative_speed();

    if (tracked.distances.empty()) {
        std::cerr << "[오류] 거리 기록 없음\n";
        return;
    }

    double current_distance = tracked.distances.back();
    double ego_velocity_mps = ego_velocity_kmph * 1000 / 3600;

    std::cout << std::fixed << std::setprecision(2)
              << "[데이터] 거리: " << current_distance << " m / 상대 속도: " << relative_speed << " m/s\n";

    // 거리 + 속도 기반 판단
    if (current_distance <= 5.0 && std::abs(relative_speed) < 0.2) {
        std::cout << "[판단] 가까운 정지 객체 → 제동\n";
        simulate_wet_braking(ego_velocity_kmph, current_distance);
    }
    else if (current_distance <= 20.0 && std::abs(relative_speed) < 1.0) {
        std::cout << "[판단] 접근 중인 느린 객체 → 감속 후 접근\n";
        simulate_wet_braking(ego_velocity_kmph, current_distance);
    }
    else {
        std::cout << "[판단] 주행 중인 객체 → 속도 맞춰 추종\n";
        match_object_speed(relative_speed);
    }
}


int main() {
    TrackedObject tracked;
    tracked.label = "car";
    double ego_velocity_kmph = 50.0; // 사용자 차량 속도
    double target_speed = 0.0;

    // 반응 시간 측정
    auto start = std::chrono::steady_clock::now();
    compute_total_braking_distance(ego_velocity_kmph * 1000 / 3600, 0, 0.35);  // 임시
    auto end = std::chrono::steady_clock::now();

    double reaction_time_sec = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
    std::cout << std::fixed << std::setprecision(6) << "반응시간: " << reaction_time_sec << "\n";
   
    for (int frame = 0; frame < FRAME_COUNT; ++frame) {
        std::cout << "\n========================\n";
        std::cout << "[프레임 " << frame << "]\n";
        auto objects = read_shared_objects(frame);
        if(objects.empty()) {
            std::cout << "[정보] 유효한 객체 없음.\n";
            continue;
        }
        // 다시 판단 (정확한 반응 시간 반영)
        calculate_braking_distance(objects[0], ego_velocity_kmph, reaction_time_sec);
        for (const auto& obj : objects) {
            update_and_decide_with_distance_and_speed(tracked, obj, ego_velocity_kmph);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    return 0;
}

