/******************************************************************************
 *
 * Module: 	TIMER0
 *
 * File Name: timer0.c
 *
 * Description: Source file for the TIMER0 AVR driver
 *
 * Author: Peak Performers
 *
 *******************************************************************************/


#include "timer0.h"
#include "std_types.h"
#include <avr/io.h> /* To use Timer0 Registers */
#include <avr/interrupt.h> /* For Timer0 ISR */

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

/* Global variables to hold the address of the call back function in the application */
static volatile void (*timer0_interrupt_callBackPtr)(void) = NULL_PTR;

/*******************************************************************************
 *                       Interrupt Service Routines                            *
 *******************************************************************************/

ISR(TIMER0_OVF_vect)
{
	if(timer0_interrupt_callBackPtr != NULL_PTR)
		{
			/* Call the Call Back function in the application after the overflow is detected */
			(*timer0_interrupt_callBackPtr)();
		}
}

ISR(TIMER0_COMP_vect)
{
	if(timer0_interrupt_callBackPtr != NULL_PTR)
		{
			/* Call the Call Back function in the application after the compare match is detected */
			(*timer0_interrupt_callBackPtr)();
		}
}

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/* Description :
 * 1. Select Normal or Compare mode based on the configuration
 * 2. Put initial value in TCNT0 register
 * 3. Select the required prescaler
 * 4. choose the compare value in case of compare mode
 */
void Timer0_init(const Timer0_ConfigType *Config_Ptr)
{   /* Open general interrupt */
	SREG |= (1<<7);
	/* Set FOC0 for non-PWM mode */
	TCCR0 |=(1<<FOC0);
	/* initial value */
	TCNT0 = Config_Ptr->initial_value ;

	/*in case of normal mode */
	if(Config_Ptr->mode == Normal)
	{
		TCCR0 &= ~(1<<WGM00) ;
		TCCR0 &= ~(1<<WGM01) ;
		/* Enable Timer0 Overflow Interrupt */
		TIMSK |= (1<<TOIE0);
	}

	/*in case of compare mode */
	else if(Config_Ptr->mode == CTC)
	{
		TCCR0|=(1<<WGM01);
		/*store compare value in OCR0 register */
		OCR0 = Config_Ptr->compare_value ;
		/*Enable Timer0 Compare Interrupt */
		TIMSK |= (1<<OCIE0);
	}

	/*select prescaler */
	TCCR0 = (TCCR0 & 0xF8) | (Config_Ptr ->prescaler);

}

/*Description :
 * we store in this function the address of the function we want to execute
 * each time there is an interrupt either this interrupt is because of overflow
 * or compare match detection
 */

void Timer0_setCallBack(void(*function_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	timer0_interrupt_callBackPtr = function_ptr;
}

/*Description :
 * function to disable timer0
 */

void Timer0_deinit(void)
{
	/* Disable Timer0 Overflow Interrupt */
	TIMSK &= ~(1<<TOIE0);
	/* Disable Timer0 Compare Interrupt */
	TIMSK &= ~(1<<OCIE0);
	/* Turn off the timer */
	TCCR0 &= ~(1<<CS00) ;
	TCCR0 &= ~(1<<CS01) ;
	TCCR0 &= ~(1<<CS02) ;


}


