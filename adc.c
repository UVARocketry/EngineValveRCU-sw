#include <xc.h>

#include "adc.h"

void adc_init() {
    ADCON0bits.ON = 1; //enable ADC
    ADCON0bits.FM = 1; //right-justify ADC result in registers

    ADCON2bits.MD = 0b011; //burst average mode
    ADCON2bits.ADCRS = 3; //shift accumulator right by 3 bits after burst average
    ADRPT = 8; //because we took 8 samples
    ADCON0bits.CONT = 1; //continuous conversion retrigger
    ADCON3bits.SOI = 1; //stop continuous retrigger (clear ADGO) on ADTIF at end of burst average

    //10us ACQ time
    //1us AD CLOCK period (Fosc/64)
    ADCLKbits.CS = 31; //1MHz ADC clock (Fadc = Fosc/2*(x+1))
    ADREFbits.PREF = 0b11; //use internal fixed voltage reference (FVR)
    ADACQL = 10; //acquisition time = 10us (internal sample/hold capacitor charges to the input voltage)

    //ADPCH selects channel
    //ADAFLTRH/L contain averaged result on ADThresholdIF
    //ADCON0bits.GO=1; //start ADC
}
//channel numbers
//0...7 RA0-RA7
//8...15 RB0-RB7
//16...23 RC0-RC7
//runtime should be about 25us (10us acq. time + 15 ADC clocks at 1MHz)
uint16_t adc_read_avg_blocking(const uint8_t channel) {
    ADPCH = channel;
    ADCON0bits.ADGO = 1; //starts burst average. also clears FILT register
    //wait for burst average done (threshold interrupt flag set in burst average mode when sample count exceeds ADRPT)
    while(!PIR1bits.ADTIF); 
    PIR1bits.ADTIF=0; //clear flag
    return (((uint16_t)ADFLTRH) << 8) | (uint16_t)ADFLTRL;
}
