#include "time.h"
#include <xc.h>

volatile uint16_t ms; //~65s before rollover
volatile uint16_t last_second_ms;
volatile uint16_t secs;

void time_init() {
    //configure 8-bit Timer0 to increment every 4us
    //and to fire an interrupt when the count reaches 255 (every 1000us),
    //so that we can maintain a millisecond count in software

    T0CON1bits.CS = 0b010; //Fosc/4=64MHz/4=16MHz
    T0CON1bits.CKPS = 0b0110; //1/64 prescaler for 0.25MHz input (T=4us)
    PIE3bits.TMR0IE = 1; //enable Timer0 interrupts
    T0CON0bits.EN = 1; //enable timer after config
}

uint16_t time_millis() {
    PIE3bits.TMR0IE = 0; //stop interrupts while accessing ms value 
    //(it's multiple bytes, so operations take several clock cycles and irq could occur)
    uint16_t result = ms;
    PIE3bits.TMR0IE = 1; //restart interrupts
    return result;
}

uint16_t time_secs() {
    PIE3bits.TMR0IE = 0; //stop interrupts while accessing ms value 
    //(it's multiple bytes, so operations take several clock cycles and irq could occur)
    uint16_t result = secs;
    PIE3bits.TMR0IE = 1; //restart interrupts
    return result;
}

//interrupt fires on Timer0 rollover

void __interrupt(irq(IRQ_TMR0)) TMR0_int() {
    PIR3bits.TMR0IF = 0; //clear flag
    //increment millis and set the flag
    ms++;
    if (ms - last_second_ms > 1000) {
        last_second_ms = ms;
        secs++;
    }
    one_kHz_flag = 1;
    //LATCbits.LC3 = ~LATCbits.LC3; //1kHz LED toggle on RC3 for test
}