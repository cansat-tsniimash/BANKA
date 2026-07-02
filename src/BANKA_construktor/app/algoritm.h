/*
 * algoritm.h
 *
 *  Created on: Apr 4, 2026
 *      Author: nikit
 */

#ifndef SRC_ALGORITM_H_
#define SRC_ALGORITM_H_

typedef enum {
    STATE_INIT = 0,
    STATE_IN_ROCKET = 1,
    STATE_FLIGHT_0 = 2,
	STATE_FLIGHT_1 = 3,
	STATE_FLIGHT_2 = 4,
	STATE_FLIGHT_3 = 5,
	STATE_BB_SEPARATE = 6
} CanSatState_t;

#endif
