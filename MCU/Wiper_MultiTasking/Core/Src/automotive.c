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

#define OBSTACLE_DIST 10 // cm

extern osMessageQueueId_t uartQueueHandle;
extern volatile uint32_t ultrasonic_center_distance_cm;
extern uint16_t current_speed;

LinePosition last_dir = LINE_CENTER;

void LineTracerDriveDecision(LinePosition dir, SensorMessage_t* msg_out)
{
    static TickType_t track_lost_time = 0;
    const TickType_t max_recovery_duration = pdMS_TO_TICKS(3000);

    uint32_t dist = ultrasonic_center_distance_cm;

    if (dist < OBSTACLE_DIST)
    {
        Motor_Stop();
        snprintf(msg_out->message, sizeof(msg_out->message),
                 "[Line] 장애물 감지 (%lu cm), 정지\r\n", dist);
        track_lost_time = 0;
        return;
    }

    switch (dir)
    {
        case LINE_ALL:
        case LINE_CENTER:
        case LINE_LEFT_CENTER:
        case LINE_RIGHT_CENTER:
            if (last_dir == LINE_LEFT)
            {
                uint16_t speed = SafeSpeed(current_speed * 0.75, 500);
                Motor_Right(speed);
                snprintf(msg_out->message, sizeof(msg_out->message), "[Line_Trace] 보정: 우회전\r\n");
            }
            else if (last_dir == LINE_RIGHT)
            {
                uint16_t speed = SafeSpeed(current_speed * 0.75, 500);
                Motor_Left(speed);
                snprintf(msg_out->message, sizeof(msg_out->message), "[Line_Trace] 보정: 좌회전\r\n");
            }
            else
            {
                Motor_Forward(current_speed);
                snprintf(msg_out->message, sizeof(msg_out->message), "[Line_Trace] 직진\r\n");
            }
            last_dir = LINE_CENTER;
            track_lost_time = 0;
            break;

        case LINE_LEFT:
            Motor_Left(current_speed);
            snprintf(msg_out->message, sizeof(msg_out->message), "[Line_Trace] 살짝 좌회전\r\n");
            last_dir = LINE_LEFT;
            track_lost_time = 0;
            break;

        case LINE_RIGHT:
            Motor_Right(current_speed);
            snprintf(msg_out->message, sizeof(msg_out->message), "[Line_Trace] 살짝 우회전\r\n");
            last_dir = LINE_RIGHT;
            track_lost_time = 0;
            break;

        default:
            if (track_lost_time == 0)
                track_lost_time = xTaskGetTickCount();

            TickType_t elapsed = xTaskGetTickCount() - track_lost_time;

            if (elapsed < max_recovery_duration)
            {
                Motor_Backward(SafeSpeed(current_speed * 0.6, 500));
                osDelay(300);
                if (last_dir == LINE_LEFT)
                    Motor_Left(SafeSpeed(current_speed, 600));
                else
                    Motor_Right(SafeSpeed(current_speed, 600));

                snprintf(msg_out->message, sizeof(msg_out->message),
                         "[Line_Trace] 라인 로스트, 복구 시도 중 (%lu ms)\r\n",
                         elapsed * portTICK_PERIOD_MS);
            }
            else
            {
                Motor_Stop();
                snprintf(msg_out->message, sizeof(msg_out->message),
                         "[Line_Trace] 복구 실패, 정지\r\n");
            }
            break;
    }
}

