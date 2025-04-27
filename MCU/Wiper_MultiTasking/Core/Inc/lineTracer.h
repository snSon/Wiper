/*
 * lineTracer.h
 *
 *  Created on: Apr 20, 2025
 *      Author: jiwan
 */

#ifndef INC_LINETRACER_H_
#define INC_LINETRACER_H_

#include <stdint.h>

// 센서 방향
typedef enum {
	LINE_NONE = 0,
	LINE_LEFT,
	LINE_CENTER,
	LINE_LEFT_CENTER,
	LINE_RIGHT_CENTER,
	LINE_RIGHT,
	LINE_ALL
} LinePosition;

void ReadLineSensor(uint8_t* left, uint8_t* center, uint8_t* right);
LinePosition DecideLineDirection(uint8_t left, uint8_t center, uint8_t right);

#endif /* INC_LINETRACER_H_ */
