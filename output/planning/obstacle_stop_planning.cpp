// Module Name : obstacle_stop_planning.cpp
// 
// Data : 2025-05-17 
// Name : LeeJunYeong

// #include "processing/object_detection"

#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

enum class DrivingAction {
    STOP,        // 정지
    DECELERATE,  // 감속
    FOLLOW,      // 추종(인식된 객체)
    CRUISE       // 유지
};

// 기본 설정값
constexpr double STOP_DISTANCE = 1.0;      // 정지 차량과 최종 거리
constexpr double SAFETY_OFFSET = 15.0;     // 속도 기반 거리 간격

DrivingAction decide_driving_action(double distance_to_object, double object_speed, double ego_speed) {
    double target_gap = std::max(ego_speed - SAFETY_OFFSET, 1.0);  // 최소 1m 확보

    // 객체가 정지 상태로 간주될 때
    if (object_speed < 0.1) {
        // 감속 시작 지점
        if (distance_to_object <= (ego_speed - SAFETY_OFFSET)) {
            return DrivingAction::DECELERATE;
        }else if (distance_to_object <= target_gap) { // 정지 차량 뒤 1m 지점에서 정지
            return DrivingAction::STOP;
        } else {
            return DrivingAction::CRUISE;  // 너무 멀리 있음, 현 상태 유지
        }
    } else {  // 객체가 주행 중
        // 사용자가 안전거리 안으로 들어왔을 때
        if (distance_to_object <= ego_speed - SAFETY_OFFSET) {
            return DrivingAction::DECELERATE;
        } else {
            return DrivingAction::FOLLOW;  // 추종 주행
        }
    }
}
