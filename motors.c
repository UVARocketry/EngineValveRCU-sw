#include "motors.h"
#include <xc.h>
#include <stdint.h>
#include "encoders.h"
#include "libpicutil/time.h"

void motors_init(void) {
    //PWM config is on p332 of datasheet
    //RB0 is Motor 1 PWM
    //RB1 is Motor 1 DIR
    //RB2 is Motor 2 PWM
    //RB3 is Motor 2 DIR

    //TMR2 is used for PWM generation. We want ~10kHz PWM (this gives ~8kHz)
    T2CLK = 0b0001; //Fclk/4
    T2CONbits.CKPS = 0b011; //1/8 prescaler
    T2CONbits.ON = 1; //start TMR2

    ANSELB &= ~0b00001111; //analog disable on pins used

    RB0PPS = 0b001001; //connect CCP1 (capture/compare 1) to RB0
    RB2PPS = 0b001010; //CCP2 to RB1

    CCP1CONbits.EN = 1; //enable CCP1
    CCP1CONbits.MODE = 0b1100; //PWM mode
    //duty cycle is 10 bits. set 0% for now
    CCPR1H = 0;
    CCPR1L = 0;

    //same config for CCP2
    CCP2CONbits.EN = 1;
    CCP2CONbits.MODE = 0b1100;
    CCPR2H = 0;
    CCPR2L = 0;

    TRISB &= ~0b00001111; //output enable
}

void motor_set(uint8_t which, int8_t speed) {
    uint16_t pwm;
    if (speed > 0) { //fwd
        if (which == 1) {
            LATBbits.LB1 = 1; //DIR is HIGH for fwd
        } else if (which == 2) {
            LATBbits.LB3 = 1;
        }
        pwm = (uint16_t) speed;
    } else {
        if (which == 1) {
            LATBbits.LB1 = 0; //DIR is LOW for rev and brake
        } else if (which == 2) {
            LATBbits.LB3 = 0;
        }
        pwm = (uint16_t) (-speed);
    }
    pwm <<= 3; //shift left to change 7-bit abs speed to 10-bit pwm signal
    if (which == 1) {
        CCPR1H = (uint8_t) (pwm >> 8);
        CCPR1L = (uint8_t) (pwm & 0xFF);
    } else if (which == 2) {
        CCPR2H = (uint8_t) (pwm >> 8);
        CCPR1L = (uint8_t) (pwm & 0xFF);
    }
}

int8_t calc_speed(uint8_t pos, uint8_t goal_pos) {
    int8_t err = (int8_t) pos - (int8_t) goal_pos;
    if (err > 5) return -127;
    if (err < -5) return 127;
    if (err > 0) return -70;
    if (err < 0) return 70;
    return 0;
}

void motor_control(Motor_t* motor) {
    motor->status.homing.limit_switch = encoders_is_limit(motor->which);
    motor->status.pos = encoders_get_pos(motor->which);

    //update warning flags
    uint16_t delta_time = time_millis() - motor->last_stopped_time;
    motor->status.faults.homing_timeout = (motor->status.homing.has_homed) && (delta_time > 5000);
    motor->status.faults.limit_switch_stuck = (motor->status.homing.limit_switch) && (motor->status.pos != 0);

    //use encoder to figure out whether motor is actually rotating
    if (motor->status.speed == 0) {
        motor->stopped_pos = motor->status.pos;
        motor-> last_stopped_time = time_millis();
    }
    
    int delta_pos = abs(motor->status.pos - motor->stopped_pos);
    
    //nonzero speed to driver but encoder doesn't rotate
    motor->status.faults.motor_stuck = (motor->status.speed != 0) && (delta_pos < 5) && (delta_time > 500);
    //zero speed to driver (brake/lock) but encoder rotates
    motor->status.faults.motor_slip_encoder_drift = (motor->status.speed == 0) && delta_pos != 0;
    motor->status.faults.encoder_error = (encoders_errors > 0);

    if (motor->status.homing.has_homed) {
        //normal operation
        //control motor speed to move toward goal position
        //simple proportional control (for now)
        //TODO: implement PID pos. ctrl.
        motor_set(motor->which, calc_speed(motor->status.pos, motor->goal_pos));
    } else {
        //we need to home the encoder
        //TODO: have valve move away from zero a bit, then close, to ensure full closure at 0-position
        motor_set(motor->which, -100); //reverse to zero position
        if (motor->status.homing.limit_switch) {
            motor->status.homing.has_homed = 1;
        }
    }
}
