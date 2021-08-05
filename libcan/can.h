/* 
 * File:   can.h
 * Author: henry
 *
 * Created on August 3, 2021, 5:49 PM
 */

#ifndef CAN_H
#define	CAN_H

#include "can_messages.h"

#define CAN_TXQ_LEN (uint8_t)8

void (*can_rx_callback)(const can_msg_t *msg);

void can_init(void);

uint8_t can_txq_push(const uint16_t id, const uint8_t len, const uint8_t *data);

#endif	/* CAN_H */

