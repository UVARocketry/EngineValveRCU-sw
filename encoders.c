#include "encoders.h"
#include <xc.h>

uint8_t enc_1_prev_state, enc_2_prev_state;

uint16_t enc_1_count;
uint16_t enc_2_count;

void encoders_init() {
    //RA0,1,2,3 are digital inputs with weak pull-ups
    //RA0 is encoder 1 phase A (interrupt)
    //RA1 is phase B
    //RA2 is encoder 2 phase A (interrupt)
    //RA3 is phase B
    //RA4 is encoder 1 limit switch (active low)
    //RA5 is encoder 2 limit switch (active low)
    ANSELA &= ~0b00111111; //disable analog
    WPUA |= 0b00111111; //enable pull up
}

//fwd (A before B): 00, 10, 11, 01, 00... (0,2,3,1,0...)
//rev (B before A): 00, 01, 11, 10, 00... (0,1,3,2,0...)
//row idx = prev_state
//col idx = next state
int8_t count_delta_matrix[] = {
    //0 1 2  3 (prev_state)
    0, 1, -1, 2, //0
    -1, 0, 2, 1, //1
    1, 2, 0, -1, //2
    2, -1, 1, 0, //3 (curr_state)
};
//off-diagonal zeroes represent invalid state transitions for a quadrature encoder,
//caused by a missed transition or some other glitch
//try sampling faster.
//todo: put another value in for these errors so they don't go undetected


//computes change to count (+1, -1, or zero) based on change in level of encoder pins since last cycle

int8_t count_delta(const uint8_t state, const uint8_t prev_state) {
    int8_t res = count_delta_matrix[state << 2 | prev_state];
    if(res==2) { //if this transition was an invalid one
        encoders_errors++; //increment error count
        return 0; //because of error, we don't know the direction of motion
    }
    return res;
}

void encoders_update() {
    uint8_t enc_1_state = (uint8_t) ((PORTAbits.RA1 << 1) | PORTAbits.RA0);
    uint8_t enc_2_state = (uint8_t) ((PORTAbits.RA3 << 1) | PORTAbits.RA2);
    enc_1_count += (uint16_t) count_delta(enc_1_state, enc_1_prev_state);
    enc_2_count += (uint16_t) count_delta(enc_2_state, enc_2_prev_state);
    enc_1_prev_state = enc_1_state;
    enc_2_prev_state = enc_2_state;
}

uint8_t encoders_is_limit(uint8_t which) {
    if (which == 1) {
        return ~PORTAbits.RA4;
    } if (which==2) {
        return ~PORTAbits.RA5;
    }
    return 0;
}

uint8_t encoders_get_pos(uint8_t which) {
    if (which == 1) {
        return (uint8_t)((float)enc_1_count * (100.0 / ENC_1_MAX_COUNT));
    } 
    if (which == 2) {
        return (uint8_t)((float)enc_2_count * (100.0 / ENC_2_MAX_COUNT));
    }
    return 0;
}
