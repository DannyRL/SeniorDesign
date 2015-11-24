/*
 * ReceiverCode.c
 *
 * Created: 11/23/2015 1:08:38 PM
 * Author : Daniel
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#define F_CPU  8000000UL

volatile int health = 100; //Local variable to keep track of health since there may be a slight delay in communicating to the central program
char Hit = 'H'; //Used to hold data sent between Xbees
char Dead = 'D';
volatile unsigned int	counts_prev;	//Input capture old value
volatile unsigned int	counts_new;		//Input capture new value
volatile unsigned int	duration;		//Duration between two capture times
unsigned int			freq;			//Variable to hold frequency
int player; //who killed you

//Used to find average duration between rising edges of signal sensed by solar cells
unsigned int total = 0; 
unsigned int sample = 0;

volatile int signal = 0; //Signal to differentiate first IC interrupt from the rest. 0 means nothing, 1 means new signal, 2 means done with sampling

void TC_init()//Initialize Timer Counter
{
	TCCR1B=0x42;	//Enable input capture, clk = F_CPU/8
}
void interrupt_init() // initialize interrupts
{
	cli();
	TIMSK1=0x20; //Input capture interrupt enable

}

void USART_init()
{
	UCSR0A = 0;
	UCSR0B = 0x98;   // Turn on RX0, TX0, and RX0 Interrupt
	UCSR0C = 0x06; // Use Async, Odd Parity, 1 Stop bit,  8-bit character sizes
	UBRR0 = 0x33; // Set baud rate to 9600
}

void playSound(int freq) //Plays sound to indicate hit or death
{
	int x = 0;
	int repeat = 0;
	
	if(health > 0){
		while( repeat != 3 ){
			x = 0;
			while(x != 200){ //play a sound for hit
				PORTA = 0b00000001;
				_delay_ms(2);
				PORTA = 0b00000000;
				_delay_ms(2);
				x++;
			}
			repeat++;
			_delay_ms(200);
		}
		UDR0 = Hit;
		return;
	} else if(health <= 0){
		UDR0 = Dead;
		while(repeat != 8){
			x = 0;
			while(x!=200){ //play a sound for hit
				PORTA = 0b00000001;
				_delay_ms(2);
				PORTA = 0b00000000;
				_delay_ms(2);
				x++;
			}
			repeat++;
			_delay_ms(200);
		}
		x = 0;
		while(x!=2200){ //Dead indicator
			PORTA = 0b00000001;
			_delay_ms(4);
			PORTA = 0b00000000;
			_delay_ms(4);
			x++;
		}
		//Determine who killed you
		player = (freq / 100);
		UDR0 = player + '0';
		
		//Wait for respawn
		_delay_ms(40000);
		x = 0;
		while(x!=1500){ //sound for respawn
			PORTA = 0b00000001;
			_delay_ms(2);
			PORTA = 0b00000000;
			_delay_ms(2);
			x++;
		}
		health = 100; //Good to go
	}
}

ISR (TIMER1_CAPT_vect)
{	
	if(sample >= 30){ //We have our sample size, get out
		signal = 2;	//Signal main that data is ready for frequency calculation
		return;
	}
	
	if(signal == 0){ //new shot, record first value and get out
		counts_prev = ICR1;	// Get new count
		signal = 1; //indicate sample size not done yet
		return;
	} 
	if(signal == 1){
		counts_new = ICR1;	// Get new count
		duration = counts_new - counts_prev; //subtract current count from last count
		total += duration; //sum all the counts within the sample size
		sample += 1; //increment sample counter
		counts_prev = counts_new; //set current IC to last IC
	}
}


int main (void)
{
	TC_init();
	interrupt_init();
	USART_init();
	DDRA = 0b00000001; //set direction of PORTA, A0 is output for speaker

	
	sei();                    // turn on interrupts
	
while (1)
{	
	if( signal == 2 )//Once signaled that we have enough samples, we calculate the frequency
	{	
		duration = total / sample; //Find average duration
		freq = (unsigned int)(99990ul / (unsigned long)duration); //calculate frequency
		
		//If hit was valid, take away health from player and go into method that plays appropriate sounds
		if(freq > 200 && freq < 260){ 
			health -= 30;
			playSound(freq);
		}
	
		signal = 0;
		total = 0;
		sample = 0;
	}
}
	
	return 0;
}
