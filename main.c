/*
 * File:   main.c
 * Author: henry
 *
 * Created on July 28, 2021, 6:10 PM
 */

#include <xc.h> //compiler

//standard C libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//code common to all RCUs
#include "libpicutil/config_mem.h" //configuration fuses for chip options
#include "libpicutil/time.h"
#include "libpicutil/uart_debug.h"
#include "libpicutil/leds.h"
#include "libcan/can.h"

//code specific to this RCU
#include "encoders.h"
#include "motors.h"
#include "solenoids.h"

const uint8_t RCU_ID_LOCAL = RCU_ID_ENGINE_VALVE_RCU;

/*
 * 
 */

uint16_t last_200Hz_time,
last_10Hz_time,
last_2Hz_time;

uint8_t connected;

struct Motor_t ox_main = {.which = 1}; //motor 1
struct Motor_t fuel_press = {.which = 2}; //motor 2

struct ValveControl_t valve_cmd;
uint8_t valve_cmd_rx_flag;

struct Heartbeat_t hb;

char msg[64];
uint16_t adcval;

void on_can_rx(const struct can_msg_t *msg);

int main() {
    INTCON0bits.GIE = 1; //enable global interrupts

    time_init();
    leds_init();
    can_rx_callback = &on_can_rx;
    can_init();
    solenoids_init();
    encoders_init();
    motors_init();

    while (1) {
        uint16_t ms = time_millis();
        if (one_kHz_flag) {
            one_kHz_flag = 0;
            encoders_update();
        }
        if (ms - last_200Hz_time > 5 || valve_cmd_rx_flag) { //200Hz or upon new valve cmd rx
            last_200Hz_time = ms;
            valve_cmd_rx_flag = 0;
            //size_t n_chars = (size_t)sprintf(msg, "E1: %6u\tE2: %6u\n\r", enc_1_count, enc_2_count);
            //uart_tx((uint8_t*)msg, n_chars);
            if (!connected) { //not connected - disregard valve_cmd struct. instead:
                valve_cmd.main_ox_valve_goal_pos = 0; //close main ox
                valve_cmd.fuel_press_valve_goal_pos = 0; //close fuel press
                valve_cmd.solenoids.engine_vent_valve_close = 0; //open engine vent
                valve_cmd.solenoids.main_fuel_valve_open = 0; //close main fuel
            }
            //critical section: receiving another ValveControl_t here would be a problem
            CAN_RX_SUSPEND();
            //copy received goal_pos to each motor's data
            ox_main.goal_pos = valve_cmd.main_ox_valve_goal_pos;
            fuel_press.goal_pos = valve_cmd.fuel_press_valve_goal_pos;
            //move the solenoids to received positions
            solenoids_set(&valve_cmd);
            CAN_RX_RESUME();


            //valve control
            motor_control(&ox_main);
            motor_control(&fuel_press);

            leds_connected(connected); //blink LED to show connection status
        }
        if (ms - last_10Hz_time > 100) { //10Hz
            last_10Hz_time = ms;
            connected = can_hb_check_connected(ms);
            //send motor status msgs for both motors
            can_txq_push(CAN_ID_MotorStatus_OX_MAIN, CAN_CONVERT(ox_main.status));
            can_txq_push(CAN_ID_MotorStatus_FUEL_PRESS, CAN_CONVERT(fuel_press.status));

            //            uint8_t arr[] = {0,0,0,0,0,0};
            //            uint16_t x = encoders_convert_pos(1,ox_main.goal_pos);
            //            uint16_t y = encoders_get_count(1);
            //            uint16_t z = y-x;
            //            arr[0] = (uint8_t)(x >> 8);
            //            arr[1] = (uint8_t)x;
            //            arr[2] = (uint8_t)(y>>8);
            //            arr[3] = (uint8_t)y;
            //            arr[4] = (uint8_t)calc_speed(y,x);
            //            can_txq_push(0x500, CAN_CONVERT(arr));
        }
        if (ms - last_2Hz_time > 500) { //2Hz
            last_2Hz_time = ms;
            //send a heartbeat msg
            hb.health = HEALTH_NOMINAL;
            hb.uptime_s = time_secs();
            can_txq_push(CAN_ID_Heartbeat, CAN_CONVERT(hb));
        }
    }
}

//on CAN frame RX
//set flag based on ID
//disable CAN RX

void on_can_rx(const struct can_msg_t *msg) {
    switch (msg->id) {
        case (CAN_ID_ValveControl | RCU_ID_MAIN_RCU): //valve control message from main RCU
            //update local ValveControl_t with data from message
            if (msg->len == sizeof (struct ValveControl_t)) {
                valve_cmd = *((struct ValveControl_t *) (msg->data));
            }
            valve_cmd_rx_flag = 1;
            break;
    }
}
