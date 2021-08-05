/* 
 * File:   motors.h
 * Author: henry
 *
 * Created on August 3, 2021, 9:45 AM
 */

#ifndef MOTORS_H
#define	MOTORS_H

#include <stdint.h>
#include "libcan/can.h"

typedef struct  {
    uint8_t which;
    MotorStatus_t status;
    uint8_t goal_pos;
    uint8_t stopped_pos;
    uint16_t last_stopped_time;
} Motor_t;

void motors_init(void);
void motor_control(Motor_t* motor);

#endif	/* MOTORS_H */

