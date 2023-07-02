
/*
 * smart_home.c
 *
 *  Created on: Apr 19, 2022
 *      Author: Peak Performers
 */

#include "smarthome32.h"

/*******************************************************************************
 *                      Global Variables                                       *
 *******************************************************************************/

uint8 test_password[NUM_DIGITS];
uint8 confirm_password[NUM_DIGITS];
uint8 password[NUM_DIGITS];
uint8 timer_counts = 0;
uint8 temp_count_button   = 0;
uint8 temp_count_temprature = 0;
uint8 button_flag = 0 ;
uint8 IR_flag = 0;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
void timerCount (void)
{
	timer_counts++;
}

void Timer1_Fast_PWM_Init(unsigned short duty_cycle)
{
	TCNT1 = 0;		/* Set timer1 initial count to zero */
	/* To get frequency = 50 Hz we use the formula F_osc= F_CPU/(1+top)*N
	 * N=64 , F_osc = 50 Hz , F_CPU = 1 MHz
	 */
	ICR1 = 2499;	/* Set TOP count for timer1 in ICR1 register */

	OCR1A = duty_cycle; /* Set the compare value */

	/* Configure timer control register TCCR1A
	 * 1. Clear OC1A on compare match (non inverting mode) COM1A1=1 COM1A0=0
	 * 2. Disconnect OC1B  COM1B0=0 COM1B1=0
	 * 3. FOC1A=0 FOC1B=0 because these bits are only active in case non-pwm mode
	 * 4. Fast Pwm Mode with the TOP in ICR1 WGM10=0 WGM11=1 (Mode Number 14)
	 */
	TCCR1A = (1<<WGM11) | (1<<COM1A1);

	/* Configure timer control register TCCR1B
	 * 1. Fast Pwm Mode with the TOP in ICR1 WGM12=1 WGM13=1 (Mode Number 14)
	 * 2. Prescaler = F_CPU/1
	 */
	TCCR1B = (1<<WGM12) | (1<<WGM13) | (1<<CS11);
}

void Open_Door(void)
{
	Timer1_Fast_PWM_Init(124);	/* Set Servo shaft at 0° position by 1 ms pulse */
	_delay_ms(500);
	Timer1_Fast_PWM_Init(250);	/* Set Servo shaft at 45° position by 1.25 ms pulse */

}

void Close_Door(void)
{

	Timer1_Fast_PWM_Init(250);	/* Set Servo shaft at 45° position by 1.25 ms pulse */
	_delay_ms(500);
	Timer1_Fast_PWM_Init(124);	/* Set Servo shaft at 0° position by 1.5 ms pulse */

}

void askForPass(void)
{
	LCD_clearScreen(); //in case their is something displayed before this message
	LCD_displayString("Enter new pass");
	LCD_moveCursor(1,0);
	for( uint8 index =0 ; index<NUM_DIGITS ; index++)
	{
		test_password[index] = UART_recieveByte();
		LCD_displayCharacter('*');
		_delay_ms(250);
	}
	LCD_clearScreen();
	LCD_displayString("Re-Enter pass");
	LCD_moveCursor(1,0);

	/* we store the 2nd trial of entering the new password in confirmation_password array
	 * to check later that there is no mismatch
	 */
	for(uint8 index =0 ; index <NUM_DIGITS ; index ++)
	{
		confirm_password[index] = UART_recieveByte();
		LCD_displayCharacter('*');
		_delay_ms(250);

	}
}

uint8 matchingCheck(void)
{
	for(uint8 index =0 ; index <NUM_DIGITS ; index++)
		{
			if( test_password[index] != confirm_password[index] )
				return 0;
		}
		return 1;
}


void main_program(void)
{
	// IR Sensor Timer Configuration
	Timer0_ConfigType IR_timer = {Normal, 0, FCPU_1024, 0};

	// ADC For Temp Sensor Configuration
	ADC_ConfigType ADC_Config = {INTERNAL_VOLTAGE, F_CPU_8};
	ADC_init (&ADC_Config);

while(1)
{
	//in case their is something displayed before this message
	uint8 temperature = LM35_getTemperature();
	LCD_moveCursor(0,0);
	LCD_displayString("Temp =      C");
	LCD_moveCursor(0,8);
	if(temperature >= 100)
	{
		LCD_intgerToString(temperature);
	}
	else
	{
		LCD_intgerToString(temperature);
		/* In case the digital value is two or one digits print space in the next digit place */
		LCD_displayCharacter(' ');
	}
	if(temperature > 45)
	{
		if(PORTB == 0x00)
		{
			temp_count_temprature = timer_counts;
			PORTB = (PORTB & 0xF0) | (0x00);
			Timer0_deinit();
			timer_counts = 0 ;
		}
		LCD_moveCursor(1,0);
		LCD_displayString("WARNING:OVERHEAT");
		PORTA |=(1<<PA1);
		Open_Door();
		Timer0_init(&IR_timer);
		Timer0_setCallBack(timerCount);

		/* F(timer) = 1 MHZ / 1024 = 1 KHz
		 * T(timer) = 1/F(timer) = 1 millisecond
		 * T(overflow) = 256*1 = 256 millisecond
		 * To count 1 minute we need 235 interrupt (60/0.256)
		 */
		while(timer_counts != 40){};
		Close_Door();
		PORTA &=~(1<<PA1);
		Timer0_deinit();
		timer_counts = 0 ;
		LCD_clearScreen();
	}
	LCD_moveCursor(1,0);
	LCD_displayString("press to open");
	uint8 counter = 0;
	if(button_flag==1)
	{
		if(PORTB == 0x00)
		{
			temp_count_button = timer_counts;
			PORTB = (PORTB & 0xF0) | (0x00);
			Timer0_deinit();
			timer_counts = 0 ;
		}
		LCD_clearScreen(); //in case their is something displayed before this message
		LCD_displayString("Enter password");
		LCD_moveCursor(1,0);
		for( uint8 index =0 ; index<NUM_DIGITS ; index++)
		{
			test_password[index] = UART_recieveByte();
			LCD_displayCharacter('*');
			_delay_ms(250);
		}
		counter++;

		while(!(matchingCheck()) && counter<3)
		{
			LCD_clearScreen(); //in case their is something displayed before this message
			LCD_displayString("Enter pass again");
			LCD_moveCursor(1,0);
			for( uint8 index =0 ; index<NUM_DIGITS ; index++)
			{
				_delay_ms(250);
				test_password[index] = UART_recieveByte();
				LCD_displayCharacter('*');
			}
			counter++;
		}
		if ((matchingCheck()) && counter==3)
			counter = 0;
		if(counter==3)
		{
			LCD_clearScreen();
			LCD_moveCursor(0,0);
			LCD_displayString("THIEF !");
			PORTA |=(1<<PA1);
			Timer0_init(&IR_timer);
			Timer0_setCallBack(timerCount);

			/* F(timer) = 1 MHZ / 1024 = 1 KHz
			 * T(timer) = 1/F(timer) = 1 millisecond
			 * T(overflow) = 256*1 = 256 millisecond
			 * To count 1 minute we need 235 interrupt (60/0.256)
			 */
			while(timer_counts != 40){//BUZZER TO BE WRITTEN
			}
			PORTA &=~(1<<PA1);
			/* once we reach time needed we turn off Timer0 & reset the global counter g_counter
			 * in case we need it in another instruction
			 */
			Timer0_deinit();
			timer_counts = 0 ;
			counter=0;
			LCD_clearScreen();

		}
		if(matchingCheck())
		{
			LCD_clearScreen();
			LCD_moveCursor(0,0);
			LCD_displayString("Gate Opened");
			Open_Door();
			Timer0_init(&IR_timer);
			Timer0_setCallBack(timerCount);

			/* F(timer) = 1 MHZ / 1024 = 1 KHz
			 * T(timer) = 1/F(timer) = 1 millisecond
			 * T(overflow) = 256*1 = 256 millisecond
			 * To count 1 minute we need 235 interrupt (60/0.256)
			 */
			while(timer_counts != 40){};
			Close_Door();
			Timer0_deinit();
			timer_counts = 0 ;
			counter = 0;
			LCD_clearScreen();
		}

	}
		button_flag = 0 ;
		/*                          IR Sensor Control                                  */

		        if (IR_flag == 1)
			 	{
			 		PORTB = (PORTB & 0xF0) | (0x00);
			 		timer_counts = 0;
			 		Timer0_init (&IR_timer);
			 		Timer0_setCallBack(timerCount);
			 		IR_flag = 0;
			 	}

		        if(temp_count_button != 0)
		        {
		        	timer_counts = temp_count_button;
		        	Timer0_init(&IR_timer);
		        	temp_count_button = 0 ;
		        }

		        if(temp_count_temprature != 0)
		        {
		        	timer_counts = temp_count_temprature;
		           	Timer0_init(&IR_timer);
		        	temp_count_temprature = 0 ;
		        }

			 	if (timer_counts >= 40)
			 	{
			 		PORTB = (PORTB & 0xF0) | (0x0F);
			 		Timer0_deinit();
			 		timer_counts = 0;
			 		temp_count_button = 0 ;
			 		temp_count_temprature = 0;
			 	}
	}

}

void INT0_init(void)
{
	SREG   &= ~(1<<7);                  // Disable interrupts by clearing I-bit
	DDRD  &= (~(1<<PD2));               // Configure INT0/PD2 as input pin
	GICR  |= (1<<INT0);                 // Enable external interrupt pin INT0
	MCUCR &= ~(1<<ISC00);
	MCUCR |= (1<<ISC01);                // Trigger INT0 with the falling edge
	SREG  |= (1<<7);                    // Enable interrupts by setting I-bit
}

void INT1_init(void)
{
	SREG   &=~(1<<7);        //Disable interrupts by clearing I-bit
	DDRD   &= ~(1<<PD3);     //Configure INT1/PD3 as input pin
	GICR   |=(1<<INT1);      //Enable external interrupt pin INT1
	MCUCR  |=(1<<ISC11);     // Trigger INT1 with the falling edge
	MCUCR  &=~(1<<ISC10);
	SREG   |=(1<<7);         // Enable interrupts by setting I-bit
}
ISR(INT0_vect)
{
	IR_flag = 1;
}

ISR(INT1_vect)
{
	button_flag = 1;
}


int main()
{
	 //LCD Initialization
	 LCD_init();

	 //UART Initialization
     /* 1. size of the character = 8 bits
      * 2. Disable parity check
      * 3. Use one stop bit
      * 4. Choose 9600 bit/sec baud rate
      */
	 UART_ConfigType UART_Configuration = { EIGHT_BIT , DISABLED , ONE_BIT , 9600 };
	 UART_init(&UART_Configuration);
	 Timer0_ConfigType IR_timer = {Normal, 0, FCPU_1024, 0};
	 INT0_init();
	 INT1_init();

	 // Ports Configuration
     DDRD &= ~(1<< PD2);
     DDRB = (DDRB & 0xF0)|(0x0F) ;
     PORTB = 0x0F;
     DDRA |=(1<<PA1);
     PORTA &=~(1<<PA1);
     DDRD |= (1<<PD5);	/* Make OC1A pin as output */

	do
 	 {
	 	askForPass();
 	 }
	 while(!( matchingCheck() ) ) ; // In case there is a mismatch between 1st & 2nd time we repeat the function
	 LCD_clearScreen();
	 main_program();

}

