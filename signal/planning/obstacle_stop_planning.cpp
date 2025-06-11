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
#include <filesystem>
// 거리 데이터 헤더파일 프레임 기준
#include "frame_distances.hpp"
const std::string LOG_DIR = "/home/hanul/Wiper_workspace/junyeong/planning/test/log_1";

const float GRAVITY = 9.8;     // 중력 가속도
const int MAX_OBJECTS = 20; // 인식 객체 최대수
const char* shm_name = "/detection_buffer";
// 실제 데이터를 기준으로 변경
const float ref_x = 940.0f;  // 기준 차의 bbox x_center
const float ref_y = 700.0f;  // 기준 차의 bbox y_center
const float tolerance = 80.0f; // 기준 허용 오차

// 공유 메모리에 저장된 구조체
// struct SharedDetectedObject {
//     float class_id;
//     float x_center;
//     float y_center;
//     float bbox_width;
//     float bbox_height;
//     float confidence;
// };

// 내부 사용을 위한 구조체
struct DetectedObject {
    float class_id;
    float x_center;
    float y_center;
    float bbox_width;
    float bbox_height;
    float confidence;
    double manual_distance = 100.0;  // 초기 거리값
};
// 변환 함수
// DetectedObject convert(const SharedDetectedObject& raw) {
//     DetectedObject obj;
//     std::memcpy(&obj, &raw, sizeof(SharedDetectedObject));
//     obj.manual_distance = 100.0;  // 수동 거리 초기화
//     return obj;
// }
// 로그 저장 함수
void save_log(int frame, const DetectedObject& obj, const std::string& decision) {
    std::ofstream log_file("/home/hanul/Wiper_workspace/changuk/planning/test/foggy0.txt", std::ios::app); // append 모드
    if (!log_file.is_open()) {
        std::cerr << "[오류] 로그 파일 열기 실패\n";
        return;
    }

    log_file << "[프레임 " << frame << "] "
             << "거리: " << std::fixed << std::setprecision(2) << obj.manual_distance << " m, "
             << "x_center: " << obj.x_center << ", "
             << "y_center: " << obj.y_center << ", "
             << "판단: " << decision << "\n";
    log_file.flush(); 
    log_file.close();
}

void save_detection_summary(const std::string& log_path, int start_frame, int end_frame, int duration) {
    std::ofstream log_file(log_path, std::ios::app);  // append 모드로 로그 파일 열기
    if (!log_file.is_open()) {
        std::cerr << "[오류] 로그 파일 열기 실패: " << log_path << "\n";
        return;
    }

    log_file << "[요약] 대상 객체 인식 시작 프레임: " << start_frame
             << ", 종료 프레임: " << end_frame
             << ", 지속 시간: " << duration << " 프레임\n";
    log_file.flush();
    log_file.close();
}

// 대상 객체인지 판별
bool is_target_object(const DetectedObject& obj) {
    return std::abs(obj.x_center - ref_x) < tolerance &&
           std::abs(obj.y_center - ref_y) < tolerance;
}
// 로그로 읽기
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
// 공유 메모리에서 객체 목록 읽기
// std::vector<DetectedObject> read_shared_objects(int frame_id) {
//     size_t OBJECT_SIZE = sizeof(DetectedObject);
//     size_t FRAME_SIZE = MAX_OBJECTS * OBJECT_SIZE;

//     int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
//     if (shm_fd == -1){
//         std::cerr << "공유 메모리 열기 실패" << std::endl;
//         exit(1);
//     }

//     void* ptr = mmap(0, FRAME_COUNT * FRAME_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
//     if (ptr == MAP_FAILED) {
//         std::cerr << "mmap 실패" << std::endl;
//         exit(1);
//     }


//     SharedDetectedObject* objects_all = static_cast<SharedDetectedObject*>(ptr);
//     SharedDetectedObject* frame_ptr = objects_all + frame_id * MAX_OBJECTS;

//     std::vector<DetectedObject> result;
//     for(int i = 0; i < MAX_OBJECTS; ++i) {
//         if (frame_ptr[i].bbox_height <= 0.0f) break;
//         result.push_back(convert(frame_ptr[i]));
//     }

//     munmap(ptr, FRAME_COUNT * FRAME_SIZE);
//     close(shm_fd);
//     return result;
// }

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
    double safe_distance = calculate_braking_distance(obj, ego_velocity_kmph, weather);

    if (distance_m + 5.0 <= safe_distance) {
        std::cout << "객체 속도에 맞춰 주행 시작... 대상 속도: " << object_speed_mps << " m/s\n";
    }
}
// 객체 속도 추정 -> 제어 신호 생성 (함수명 바꾸자)
void control_signal(const DetectedObject& obj, double ego_velocity_kmph, int weather, int frame) {
    // 인식된 객체 속도 0으로 함
    double relative_speed = 0.0f;

    // 안전거리
    double safe_distance = calculate_braking_distance(obj, ego_velocity_kmph, weather);
    // 객체와의 거리
    double distance_m = obj.manual_distance;
    
    std::string decision;
    // 거리 + 속도 기반
    if (distance_m <= (safe_distance + 5.0) && std::abs(relative_speed) < 1.0) {
        decision = "감속 후 정지";
        std::cout << "[판단] 전방에 멈춘 객체 인식 → 감속 후 정지 시작\n";
        // simulate_braking(ego_velocity_kmph, distance_m);
    }
    else {
        // decision = "속도 맞춰 추종";
        // std::cout << "[판단] 주행 중인 객체 → 속도 맞춰 추종\n";
        // match_object_speed(obj, ego_velocity_kmph, relative_speed, weather);
    }
    if (!decision.empty()) {
    std::cout << std::fixed << std::setprecision(2)
              << "[데이터] 거리: " << distance_m << " m / 제동 안전 거리: " << safe_distance << " m\n";
    std::cout << "[판단] " << decision << "\n";
    save_log(frame, obj, decision);
}
}


int main() {
    const int frame_interval_ms = 33;  // 목표 간격 (30 FPS)
    const std::string summary_log_path = "/home/hanul/Wiper_workspace/changuk/planning/test/foggy0_detection.txt";

    int FRAME_COUNT = 0;
    for (const auto& entry : std::filesystem::directory_iterator(LOG_DIR)) {
        if (entry.is_regular_file()) {
            FRAME_COUNT++;
        }
    }

    int detection_start_frame = -1;  // 대상 객체가 처음 인식된 프레임
    int detection_end_frame = -1;    // 대상 객체가 마지막으로 인식된 프레임
    int detection_duration = 0;      // 대상 객체가 지속적으로 탐지된 프레임 수

    for (int frame = 0; frame < FRAME_COUNT; ++frame) {
        auto start = std::chrono::steady_clock::now();  // 프레임 시작 시각

        std::cout << "\n========================\n";
        std::cout << "[프레임 " << frame << "]\n";

        auto objects = load_objects_from_log(frame);

        auto logic_start = std::chrono::steady_clock::now();

        if (!objects.empty()) {
            bool object_recognized = false;  // 객체 인식 여부 플래그

            for (auto& obj : objects) {
                if (is_target_object(obj)) {
                    object_recognized = true;
                    auto it = frame_to_distance.find(frame);
                    if (it != frame_to_distance.end()) {
                        obj.manual_distance = it->second;
                    }

                    int weather = 1;  // 0 = 맑음, 1 = 비
                    control_signal(obj, 60.0f, weather, frame);

                    // 로그 작성
                    save_log(frame, obj, "대상 객체 인식");
                }
            }

            if (object_recognized) {
                std::cout << "[정보] 제어 신호 준비 완료.\n";

                // 대상 객체가 처음 인식된 프레임 기록
                if (detection_start_frame == -1) {
                    detection_start_frame = frame;
                }

                // 지속적으로 탐지된 프레임 수 증가
                detection_duration++;
                detection_end_frame = frame;  // 마지막으로 인식된 프레임 갱신
            } else {
                std::cout << "[정보] 대상 객체 없음.\n";
            }
        } else {
            std::cout << "[정보] 유효한 객체 없음.\n";

            // 대상 객체가 사라졌다면 요약 로그 작성
            if (detection_start_frame != -1 && detection_end_frame != -1) {
                save_detection_summary(summary_log_path, detection_start_frame, detection_end_frame, detection_duration);

                // 상태 초기화
                detection_start_frame = -1;
                detection_end_frame = -1;
                detection_duration = 0;
            }
        }

        auto logic_end = std::chrono::steady_clock::now();
        auto logic_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(logic_end - logic_start).count();

        // 프레임 처리 시간 측정
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        int sleep_time = frame_interval_ms - elapsed;
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }

        auto total_end = std::chrono::steady_clock::now();
        auto total_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - start).count();
    }

    // 마지막으로 대상 객체가 인식된 상태라면 요약 로그 작성
    if (detection_start_frame != -1 && detection_end_frame != -1) {
        save_detection_summary(summary_log_path, detection_start_frame, detection_end_frame, detection_duration);
    }

    return 0;
}



