/* 
 * File:   encoders.h
 * Author: henry
 *
 * Created on August 2, 2021, 12:24 PM
 */

#ifndef ENCODERS_H
#define	ENCODERS_H

#include <stdint.h>

#define ENC_1_MAX_COUNT 1000
#define ENC_2_MAX_COUNT 500

uint8_t encoders_errors;
uint16_t enc_1_count;
uint16_t enc_2_count;

void encoders_init(void);
void encoders_update(void);

uint8_t encoders_is_limit(uint8_t which);
uint8_t encoders_get_pos(uint8_t which);

#endif	/* ENCODERS_H */

