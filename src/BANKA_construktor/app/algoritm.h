/*
 * algoritm.h
 *
 *  Created on: Apr 4, 2026
 *      Author: nikit
 */

#ifndef SRC_ALGORITM_H_
#define SRC_ALGORITM_H_
#endif



typedef enum {
    STATE_INIT = 0,
    STATE_IN_ROCKET = 1,
    STATE_FLIGHT = 2,
	STATE_BB_SEPARATE = 3,
	STATE_ON_GROUND = 4
} CanSatState_t;
