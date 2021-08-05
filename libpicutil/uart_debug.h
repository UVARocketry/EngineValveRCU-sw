/* 
 * File:   serial_debug.h
 * Author: henry
 *
 * Created on August 2, 2021, 7:47 AM
 */

#ifndef UART_DEBUG_H
#define	UART_DEBUG_H

#include <stdlib.h>
#include <stdint.h>

#define UART_TXBUF_SIZE (uint8_t)64 //bytes of RAM to use for TX buffer

void uart_init(void);
void uart_tx(uint8_t* bytes, size_t n_bytes);

#endif	/* SERIAL_DEBUG_H */

