/*
 * common.h
 *
 * Created: 06-01-2017 07:21:27
 *  Author: JP
 */ 


#ifndef COMMON_H_
#define COMMON_H_

/*
 * Common declarations.
 * Routines and variables which are used in all modules.
 */

/* !!! IMPORTANT !!! always define F_CPU before including any AVR headers */
#define F_CPU		16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "utility.h"

typedef uint8_t boolean;

#define TRUE  (uint8_t)1
#define FALSE 0

#define NULL  0

#endif /* COMMON_H_ */