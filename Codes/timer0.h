/******************************************************************************
 *
 * Module: 	TIMER0
 *
 * File Name: timer0.h
 *
 * Description: Header file for the TIMER0 AVR driver
 *
 * Author: Peak Performers
 *
 *******************************************************************************/

#ifndef TIMER0_H_
#define TIMER0_H_
#include "std_types.h"

/*******************************************************************************
 *                         Types Declaration                                   *
 *******************************************************************************/
typedef enum
{
	Normal , CTC
} Timer_mode ;

typedef enum
{
	NO_PRESCALING = 1 , FCPU_8 , FCPU_64 , FCPU_256 , FCPU_1024
} Prescaler ;

typedef struct
{
	Timer_mode mode ;
	uint8 initial_value ;
	Prescaler prescaler ;
	uint8 compare_value ;
} Timer0_ConfigType ;


/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/* Description :
 * 1. Select Normal or Compare mode based on the configuration
 * 2. Put initial value in TCNT0 register
 * 3. Select the required prescaler
 * 4. choose the compare value in case of compare mode
 */
void Timer0_init(const Timer0_ConfigType *Config_Ptr);

/*Description :
 * we store in this function the address of the function we want to execute
 * each time there is an interrupt either this interrupt is because of overflow
 * or compare match detection
 */
void Timer0_setCallBack(void(*function_ptr)(void));

/*Description :
 * function to disable timer0
 */
void Timer0_deinit(void);


#endif /* TIMER0_H_ */
