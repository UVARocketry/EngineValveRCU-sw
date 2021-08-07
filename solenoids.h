/* 
 * File:   solenoids.h
 * Author: henry
 *
 * Created on August 3, 2021, 10:10 AM
 */

#ifndef SOLENOIDS_H
#define	SOLENOIDS_H


#include <stdint.h>
#include "libcan/can.h"

void solenoids_init(void);
void solenoids_set(struct SolenoidsControl_t* cmd);

#endif	/* SOLENOIDS_H */

