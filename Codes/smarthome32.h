/*
 * smarthome32.h
 *
 *  Created on: Apr 22, 2022
 *      Author: Peak Performers
 */

#ifndef SMARTHOME32_H_
#define SMARTHOME32_H_

#include "std_types.h"
#include "lcd.h"
#include "uart.h"
#include "timer0.h"
#include "lm35_sensor.h"
#include "adc.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/*******************************************************************************
 *                              Definitions                                    *
 *******************************************************************************/

#define NUM_DIGITS 5

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*  Description :
 *  Ask the user to enter his new password
 */

void askForPass (void);

/* Description :
 *  Check if there is a mismatch between the password and confirmation password
 */

uint8 matchingCheck(void);

/* Description :
 * 1. We store the address of this function in ISR of Timer0 so it increments a global counter (g_counter)
 *    each time there is an interrupt
 * 2. this function does'nt stop incrementing until we turn off Timer0 interrupts
 * 3. we turn off Timer0 interrupts when we reach the time needed and that depends on the function
 *    that opens Timer0 interrupts
 */

void timerCount (void);
void main_program(void);
void INT2_init(void);
void timerCount (void);
void Timer1_Fast_PWM_Init(unsigned short duty_cycle);
void Open_Door(void);
void Close_Door(void);

#endif /* SMART_HOME_H_ */



