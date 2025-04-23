/*
 * automotive.h
 *
 *  Created on: Apr 23, 2025
 *      Author: gg065
 */

#ifndef INC_AUTOMOTIVE_H_
#define INC_AUTOMOTIVE_H_

#include "main.h"
#include "task_manage.h"
#include "lineTracer.h"

uint16_t SafeSpeed(uint16_t desired, uint16_t min_required);
void LineTracerDriveDecision(LinePosition dir, SensorMessage_t* msg_out);

#endif /* INC_AUTOMOTIVE_H_ */
