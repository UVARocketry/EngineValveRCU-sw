#include <xc.h>
#include <stdint.h>
#include "solenoids.h"

void solenoids_init(void) {
    //RA6,7 and RB4,5 are solenoid control pins connected to MOSFET switches
    //RB4 is solenoid 1 (engine_vent_valve_close)
    //RB5 is solenoid 2 (main_fuel_valve_open)
    //RA6 is solenoid 3 (extra)
    //RA7 is solenoid 4 (extra)
    ANSELBbits.ANSELB4 = ANSELBbits.ANSELB5 = 0; //no analog functions
    ANSELAbits.ANSELA6 = ANSELAbits.ANSELA7 = 0;
    TRISBbits.TRISB4 = TRISBbits.TRISB5 = 0; //output enable
    TRISAbits.TRISA6 = TRISAbits.TRISA7 = 0;
}

void solenoids_set(struct ValveControl_t* cmd) {
    LATBbits.LB4 = cmd->solenoids.engine_vent_valve_close;
    LATBbits.LB5 = cmd->solenoids.main_fuel_valve_open;
    LATAbits.LA6 = cmd->solenoids.aux_1_energize;
    LATAbits.LA7 = cmd->solenoids.aux_2_energize;
}
