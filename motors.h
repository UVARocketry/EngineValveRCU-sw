/* 
 * File:   motors.h
 * Author: henry
 *
 * Created on August 3, 2021, 9:45 AM
 */

#ifndef MOTORS_H
#define	MOTORS_H

#include <stdint.h>
#include "libcan/can_messages.h"
#include "libcan/can.h"

struct Motor_t {
    uint8_t which;
    struct MotorStatus_t status;
    uint8_t goal_pos;
    uint8_t stopped_pos;
    uint16_t last_stopped_time;
};

void motors_init(void);
void motor_control(struct Motor_t* motor);

int8_t calc_speed(uint16_t pos, uint16_t goal_pos); //delete from header

#endif	/* MOTORS_H */

