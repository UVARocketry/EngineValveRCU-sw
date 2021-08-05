/* 
 * File:   adc.h
 * Author: henry
 *
 * Created on August 5, 2021, 8:43 AM
 */

#ifndef ADC_H
#define	ADC_H

void adc_init(void);
uint16_t adc_read_avg_blocking(const uint8_t channel);

#endif	/* ADC_H */

