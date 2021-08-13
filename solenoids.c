#include <xc.h>
#include <stdint.h>
#include "solenoids.h"

void solenoids_init(void) {
    //solenoid control pins connected to MOSFET switches
    //RB4 is solenoid 1 (engine_vent_valve_close)
    //RB5 is solenoid 2 (main_fuel_valve_open)
    //RC4 is solenoid 3 (extra)
    //RC5 is solenoid 4 (extra)
    ANSELB4 = 0; //disable analog
    ANSELB5 = 0;
    ANSELC4 = 0;
    ANSELC5 = 0;
    TRISB4 = 0; //enable output
    TRISB5 = 0;
    TRISC4 = 0;
    TRISC5 = 0;
}

void solenoids_set(struct ValveControl_t* cmd) {
    LATB4 = cmd->solenoids.engine_vent_valve_close;
    LATB5 = cmd->solenoids.main_fuel_valve_open;
    LATC4 = cmd->solenoids.aux_1_energize;
    LATC5 = cmd->solenoids.aux_2_energize;
}
