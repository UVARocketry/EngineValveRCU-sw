/* 
 * File:   can_messages.h
 * Author: henry
 *
 * Created on August 3, 2021, 10:34 AM
 */

#ifndef CAN_MESSAGES_H
#define	CAN_MESSAGES_H

#include <stdint.h>

typedef struct {
    // Standard Identifier - 11 bits long
    uint16_t id;
    // How many bytes are used in data
    uint8_t len;
    // the data you want to transmit
    uint8_t data[8];
} can_msg_t;

//Message ID numbers (11-bit: 0x000 to 0x7FF)
//Lower numbers get priority if there is a bus contention, so use lower numbers
//for more critical data
//The lower 4 bits are a RCU ID (up to 16 RCUs on bus),
//so that message IDs go in increments of 0x10 or 16
//RCU ID numbers (4-bit: 0x00 to 0x0F, 0 to 15)
#define RCU_ID_MAIN_RCU 1
#define RCU_ID_ENGINE_VALVE_RCU 2
#define RCU_ID_ENGINE_SENSOR_RCU 3
#define RCU_ID_POWER_PYRO_RCU 4

#define ID_PYRO_CONTROL 0x50
typedef struct {
    uint8_t fire_ignitor:1;
    uint8_t fire_main:1;
    uint8_t fire_drogue:1;
} PyroControl_t;

#define ID_VALVE_CONTROL 0x100
typedef struct {
    uint8_t engine_vent_valve_close : 1;
    uint8_t main_fuel_valve_open : 1;
    uint8_t solenoid_3_energize : 1;
    uint8_t solenoid_4_energize : 1;
} SolenoidsControl_t;
typedef struct {
    uint8_t main_ox_valve_goal_pos;
    uint8_t fuel_pres_valve_goal_pos;
    SolenoidsControl_t solenoids;
} ValveControl_t;

#define ID_ENGINE_SENSORS_A 0x110
typedef struct {
    uint16_t thrust_raw;
    uint16_t chamber_press_1_raw;
    uint16_t chamber_press_2_raw;
    uint16_t chamber_press_3_raw;
} EngineSensorsA_t;

#define ID_ENGINE_SENSORS_B 0x120
typedef struct {
    uint16_t fuel_inj_press_raw;
    uint16_t ox_inj_press_raw;
    uint16_t fuel_tank_press_raw;
    uint16_t aux_raw;
} EngineSensorsB_t;

#define ID_OX_MAIN_MOTOR_STATUS 0x200
#define ID_FUEL_PRESS_MOTOR_STATUS 0x210
typedef struct {
    uint8_t pos;
    int8_t speed;
    struct LimitSwitchStatus_t {
        uint8_t limit_switch : 1;
        uint8_t has_homed : 1;
    } homing;
    struct MotorValveFaults_t {
        uint8_t motor_stuck : 1;
        uint8_t homing_timeout : 1;
        uint8_t limit_switch_stuck : 1;
        uint8_t motor_slip_encoder_drift : 1;
        uint8_t encoder_error : 1;
    } faults;
} MotorStatus_t;

#define ID_PYRO_STATUS 0x220
typedef struct {
    uint8_t ignitor_cont : 1;
    uint8_t main_cont : 1;
    uint8_t drogue_cont : 1;
} PyroStatus_t;

#define ID_POWER_STATUS 0x230
typedef struct {
    uint16_t voltage_12V_mV;
    uint16_t current_12V_mA;
    uint16_t current_5V_mA;
    struct {
        uint8_t external_power_connected:1;
        uint8_t undervoltage:1;
        uint8_t overvoltage:1;
        uint8_t overcurrent_12V:1;
        uint8_t overcurrent_5V:1;
        uint8_t shutdown_imminent:1;
        uint8_t valve_fuse_blown:1;
    } flags;
} PowerStatus_t;

#define ID_POWER_CONTROL 0x230
typedef struct {
    uint8_t shutdown_request:1;
} PowerControl_t;

#define ID_HEARTBEAT 0x400
typedef struct {
    uint8_t health;
    uint16_t uptime_s;
} Heartbeat_t;
#define HEALTH_NOMINAL 0
#define HEALTH_WARNING 1
#define HEALTH_CRITICAL 2

#define ID_ERROR 0x410
typedef struct {
    uint8_t error_no;
    uint8_t error_data[7];
} Error_t;

#define ID_DEBUG_PRINT 0x420
typedef struct {
    char* text[8];
} DebugPrint_t;

#endif	/* CAN_MESSAGES_H */

