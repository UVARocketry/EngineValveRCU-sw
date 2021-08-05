/* 
 * File:   time.h
 * Author: henry
 *
 * Created on August 2, 2021, 5:18 PM
 */

#ifndef TIME_H
#define	TIME_H

#include <stdint.h>
#include <xc.h>

void time_init(void);
uint16_t time_millis(void);
uint16_t time_secs(void);

uint8_t one_kHz_flag;

#endif	/* TIME_H */

