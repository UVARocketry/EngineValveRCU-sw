#include <xc.h>
#include <stdint.h>
#include "solenoids.h"

void solenoids_init(void) {
    //RA6,7 and RB4,5are solenoid control pins connected to MOSFET switches
    //RB4 is solenoid 1 (engine_vent_valve_close)
    //RB5 is solenoid 2 (main_fuel_valve_open)
    //RA6 is solenoid 3 (extra)
    //RA7 is solenoid 4 (extra)
    ANSELBbits.ANSELB4 = ANSELBbits.ANSELB5 = 0; //no analog functions
    ANSELAbits.ANSELA6 = ANSELAbits.ANSELA7 = 0;
    TRISBbits.TRISB4 = TRISBbits.TRISB5 = 0; //output enable
    TRISAbits.TRISA6 = TRISAbits.TRISA7 = 0;
}

void solenoids_set(SolenoidsControl_t* cmd) {
    LATBbits.LB4 = cmd->engine_vent_valve_close;
    LATBbits.LB5 = cmd->main_fuel_valve_open;
    LATBbits.LB6 = cmd->solenoid_3_energize;
    LATBbits.LB7 = cmd->solenoid_4_energize;
}
