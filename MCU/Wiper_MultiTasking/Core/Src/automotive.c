/*
 * automotive.c
 *
 *  Created on: Apr 23, 2025
 *      Author: gg065
 */

#include "automotive.h"
#include "motor.h"
#include "lineTracer.h"
#include "ultrasonic.h"
#include "bluetooth.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

#define OBSTACLE_DIST 30 // cm

extern osMessageQueueId_t uartQueueHandle;
extern volatile uint32_t ultrasonic_center_distance_cm;
extern uint16_t current_speed; // 현재 설정된 속도
uint16_t recovery_speed=800; // 복구 속도

LinePosition last_dir = LINE_CENTER; // 마지막 방향 저장 (직진 보정 시 참조)

/*
 * 라인 트레이서 방향에 따른 주행 결정 함수
 * dir : 라인센서로부터 판단된 현재 방향
 * msg_out : UART 출력 메시지 포인터
 */

void LineTracerDriveDecision(LinePosition dir, SensorMessage_t* msg_out)
{
    static TickType_t track_lost_time = 0;
    const TickType_t max_recovery_duration = pdMS_TO_TICKS(3000); // 3 sec

    uint32_t dist = ultrasonic_center_distance_cm;

    // 장애물이 앞에 있을 경우 정지
    if (dist < OBSTACLE_DIST)
    {
        Motor_Stop();
        snprintf(msg_out->message, sizeof(msg_out->message),
                 "[Line] 장애물 감지 (%lu cm), 정지\r\n", dist);
        track_lost_time = 0;
        return;
    }

    // 방향에 따른 주행 동작 결정
    switch (dir)
    {
    	// 직진 계열 (라인이 중앙 또는 좌우 중심에 있음)
        case LINE_ALL:
        case LINE_CENTER:
        	Motor_Forward(current_speed);
        	snprintf(msg_out->message, sizeof(msg_out->message), "[Line_Trace] 직진\r\n");
            last_dir = LINE_CENTER; // 방향 갱신
            track_lost_time = 0;
            break;

        // 왼쪽 센서만 라인을 감지
        case LINE_LEFT:
            Motor_Left(current_speed);
            snprintf(msg_out->message, sizeof(msg_out->message), "[Line_Trace] 보정 좌회전\r\n");
            last_dir = LINE_LEFT;
            track_lost_time = 0;
            break;

        // 우측 센서만 라인을 감지
        case LINE_RIGHT:
            Motor_Right(current_speed);
            snprintf(msg_out->message, sizeof(msg_out->message), "[Line_Trace] 보정 우회전\r\n");
            last_dir = LINE_RIGHT;
            track_lost_time = 0;
            break;

        // 라인을 아예 감지하지 못한 경우 (라인 로스트)
        default:
        	// 처음 감지 못한 시각 저장
            if (track_lost_time == 0)
                track_lost_time = xTaskGetTickCount();

            TickType_t elapsed = xTaskGetTickCount() - track_lost_time;

            // 1. 1초 미만이면, 후진 없이 좌/우 회전만 시도
            if (elapsed < pdMS_TO_TICKS(1000))
            {
            	if (last_dir == LINE_LEFT)
					Motor_Left(current_speed);
				else
					Motor_Right(current_speed);

            	 snprintf(msg_out->message, sizeof(msg_out->message),
							 "[Line_Trace] 라인 로스트 (%.1fs), 회전 복구 시도 중\r\n",
							 elapsed * 1.0f / configTICK_RATE_HZ);
            }

            // 2. 1~3초 사이라면, 후진 후 회전 시도
            else if (elapsed < max_recovery_duration)
			{
				Motor_Backward(current_speed);
				osDelay(300);

				if (last_dir == LINE_LEFT)
					Motor_Left(current_speed);
				else
					Motor_Right(current_speed);

				snprintf(msg_out->message, sizeof(msg_out->message),
						 "[Line_Trace] 라인 로스트 (%.1fs), 후진+회전 복구 시도\r\n",
						 elapsed * 1.0f / configTICK_RATE_HZ);
			}

            // 3. 3초 초과 시 정지
			else
			{
				Motor_Stop();
				snprintf(msg_out->message, sizeof(msg_out->message),
						 "[Line_Trace] 복구 실패 (%.1fs), 정지\r\n",
						 elapsed * 1.0f / configTICK_RATE_HZ);
				track_lost_time = 0; // 다음을 위해 리셋
			}

            break;
    }
}

