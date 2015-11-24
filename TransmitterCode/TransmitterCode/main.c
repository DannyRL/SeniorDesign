#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#define F_CPU  8000000UL
#define LCD_DATA 0x250001

void lcd_init();
void lcd_command(char);
void lcd_char(char);
void interrupt_init();

uint16_t resolution;
unsigned int j;

int player[6] = {100,1200,0,0,0,0}; //Global array holding player health, ammo, kills, deaths, and team 1 and 2 scores -- respectively
volatile int stat = 0; //used to determine which stats to show
volatile int check = 0; //used to check if player wants to see different stats. Will clear screen if stat != check
char ReceivedByte;
int times;

void lcd_init() //Initializes LCD
{
	DDRC |= 0xCF;
	//Change DDRA--> DDRB and PORTA--> PORTB if LCD attached to port B and so forth
	lcd_command(0x33);
	//Initialize LCD Driver
	lcd_command(0x32);
	//Four bit mode
	lcd_command(0x2C);
	//2 Line Mode
	lcd_command(0x0C);
	//Display On, Cursor Off, Blink Off Change to 0x0F if cursor is desired
	lcd_command(0x01);
	//Clear Screen, Cursor Home
}

void lcd_command(char cmd) //Sends commands to LCD
{
	char temp = cmd;
	PORTC=0;
	_delay_ms(5);
	cmd = ( (cmd & 0xF0) >> 4) | 0x80;
	//Write Upper Nibble (RS=0) E--> 1
	PORTC = cmd;
	_delay_ms(5);
	cmd ^= 0x80; //E--> 0
	PORTC = cmd;
	_delay_ms(5);
	cmd = temp;
	cmd = ( (cmd & 0x0F) ) | 0x80;
	//Write Lower Nibble (RS=0) E--> 1
	PORTC = cmd;
	_delay_ms(5);
	cmd ^= 0x80; //E--> 0
	PORTC = cmd;
	_delay_ms(7);
}

void lcd_char(char data) //Writes a character to LCD
{
	char temp = data;
	//PORTD = 0x40;
	PORTC = 0x40; //enable = bit 7, rs = bit 6, bit 5 and bit 4 are blank
	_delay_ms(5);
	data = ( (data & 0xF0) >> 4) | 0xC0;
	//Write Upper Nibble (RS=1) E--> 1
	PORTC = data;
	_delay_ms(5);
	data ^= 0x80; // E--> 0
	PORTC = data;
	_delay_ms(5);
	data = temp;
	data = ( (data & 0x0F) ) | 0xC0;
	//Write Lower Nibble (RS=1) E--> 1
	PORTC = data;
	_delay_ms(5);
	data ^= 0x80;
	//E--> 0
	PORTC = data;
	_delay_ms(7);
}

int scale(int mag) //determines magnitude of the number so it can be sent to the LCD correctly
{ 
	if(mag<10){
		return 1;
	}

	else if(mag < 100 && mag >= 10){
		return 2;
	}
	
	else if(mag>=100 && mag <= 999){
		return 3;
	}

	else if(mag <=9999 && mag >=1000){
		return 4;
	}else return 4;
}

void healthAmmo(int health, int ammo) //Prints player health and ammo
{
	int num; //num is digit value
	int r; //r is remainder, used in next case
	int temp;
	char cNumber; //character to pass to lcd_char

	int scale1 = scale(health);
	int scale2 = scale(ammo);

	//Print health
	r = health;

	lcd_char('H');
	lcd_char('P');
	lcd_char(':');
	lcd_char(' ');
	if(scale1 == 2){
		lcd_char(' ');
	}else if(scale1 == 1){
		lcd_char(' ');
		lcd_char(' ');
	}

	switch(scale1) //prints out health in appropriate scale
	{

		case 4: //1k digit
		num = r/1000;
		r = r%1000;
		temp = num + (int)'0';
		cNumber = (char)temp;
		lcd_char(cNumber);

		case 3: //100 digit
		num = r/100;
		r = r%100;
		temp = num + (int)'0';
		cNumber = (char)temp;
		lcd_char(cNumber);
		case 2: //10 digit
		num = r/10;
		r = r%10;
		temp = num + (int)'0';
		cNumber = (char)temp;
		lcd_char(cNumber);

		case 1: //1 digit
		num = r/1;
		temp = num + (int)'0';
		cNumber = (char)temp;
		lcd_char(cNumber);

		break;


	}
	lcd_command(0xC0);

	//Print back
	r = ammo; //r is remainder, used in next case

	lcd_char('A');
	lcd_char('M');
	lcd_char('M');
	lcd_char('O');
	lcd_char(':');
	lcd_char(' ');
	if(scale2 == 1){
		lcd_char(' ');
	}


	switch(scale2) //prints out ammo in appropriate scale
	{
		case 4: //1k digit
		num = r/1000;
		r = r%1000;
		temp = num + (int)'0';
		cNumber = (char)temp;
		lcd_char(cNumber);

		case 3: //100 digit
		num = r/100;
		r = r%100;
		temp = num + (int)'0';
		cNumber = (char)temp;
		lcd_char(cNumber);
		case 2: //10 digit
		num = r/10;
		r = r%10;
		temp = num + (int)'0';
		cNumber = (char)temp;
		lcd_char(cNumber);

		case 1: //1 digit
		num = r/1;
		temp = num + (int)'0';
		cNumber = (char)temp;
		lcd_char(cNumber);

		break;


	}
	lcd_char('/');
	lcd_char('1');
	lcd_char('2');
}

void killDeath(int kills, int deaths) //Prints player kills and deaths
{
		int num; //num is digit value
		int r; //r is remainder, used in next case
		int temp;
		char cNumber; //character to pass to lcd_char

		int scale1 = scale(kills);
		int scale2 = scale(deaths);

		//Print health
		r = kills;

		lcd_char('K');
		lcd_char('I');
		lcd_char('L');
		lcd_char('L');
		lcd_char('S');
		lcd_char(':');
		lcd_char(' ');

		switch(scale1) //prints out health in appropriate scale
		{

			case 4: //1k digit
			num = r/1000;
			r = r%1000;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			case 3: //100 digit
			num = r/100;
			r = r%100;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);
			case 2: //10 digit
			num = r/10;
			r = r%10;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			case 1: //1 digit
			num = r/1;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			break;


		}
		lcd_command(0xC0);

		//Print back
		r = deaths; //r is remainder, used in next case

		lcd_char('D');
		lcd_char('E');
		lcd_char('A');
		lcd_char('T');
		lcd_char('H');
		lcd_char('S');
		lcd_char(':');
		lcd_char(' ');

		switch(scale2) //prints out ammo in appropriate scale
		{
			case 4: //1k digit
			num = r/1000;
			r = r%1000;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			case 3: //100 digit
			num = r/100;
			r = r%100;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);
			case 2: //10 digit
			num = r/10;
			r = r%10;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			case 1: //1 digit
			num = r/1;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			break;
		}
	}
	
void teamScores(int one, int two)//Prints team scores
{
		int num; //num is digit value
		int r; //r is remainder, used in next case
		int temp;
		char cNumber; //character to pass to lcd_char

		int scale1 = scale(one);
		int scale2 = scale(two);
	

		//Print health
		r = one;

		lcd_char('T');
		lcd_char('E');
		lcd_char('A');
		lcd_char('M');
		lcd_char(' ');
		lcd_char('1');
		lcd_char(':');
		lcd_char(' ');

		switch(scale1) //prints out health in appropriate scale
		{

			case 4: //1k digit
			num = r/1000;
			r = r%1000;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			case 3: //100 digit
			num = r/100;
			r = r%100;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);
			case 2: //10 digit
			num = r/10;
			r = r%10;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			case 1: //1 digit
			num = r/1;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			break;


		}
		lcd_command(0xC0);

		//Print back
		r = two; //r is remainder, used in next case

		lcd_char('T');
		lcd_char('E');
		lcd_char('A');
		lcd_char('M');
		lcd_char(' ');
		lcd_char('2');
		lcd_char(':');
		lcd_char(' ');

		switch(scale2) //prints out ammo in appropriate scale
		{
			case 4: //1k digit
			num = r/1000;
			r = r%1000;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			case 3: //100 digit
			num = r/100;
			r = r%100;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);
			case 2: //10 digit
			num = r/10;
			r = r%10;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			case 1: //1 digit
			num = r/1;
			temp = num + (int)'0';
			cNumber = (char)temp;
			lcd_char(cNumber);

			break;


		}
	}

void showInfo() //Determines what info to display on screen
{
	if(check != stat){
		lcd_command(0x01); //Clear Screen so display won't look weird
	}
	
	lcd_command(0x02); // Cursor Home
	_delay_ms(5);
	
	//Go into appropriate method to display data
	if(stat == 0){
		check = stat;
		healthAmmo(player[0], player[1]);
	} else if(stat == 1){
		check = stat;
		killDeath(player[2], player[3]);
	} else if(stat == 2){
		check = stat;
		teamScores(player[4], player[5]);
	}
}

void interrupt_init() // initialize interrupts
{
	cli();
	EIMSK = 0x03;
	EICRA = 0x0F;				// set INT0 and INT1 to trigger on rising edge
}

void USART_init()
{
	UCSR0A = 0;
	UCSR0B = 0x98;   // Turn on RX0, TX0, and RX0 Interrupt
	UCSR0C = 0x06; // Use Async, Odd Parity, 1 Stop bit,  8-bit character sizes

	UBRR0 = 0x33; // Set baud rate to 9600
}


ISR (INT0_vect) //ISR for shot fired
{
		if(player[1] <= 0){ //reload
			_delay_ms(20000);
			player[1] = 12;
			EIFR = (1 << INTF0);
			return;
		}
		if(player[0] == 0){
			EIFR = (1 << INTF0);
			return;
		}
		
		player[1] -= 1; 
		int x = 0;
		while(x!=50){ //oscillating laser for player ID
			PORTA = 0b00001100;
			_delay_ms(20);
			PORTA = 0b00001000;
			_delay_ms(20);
			x++;
		}
		EIFR = (1 << INTF0);
}

ISR (INT1_vect) //ISR for switch though stats
{
	stat += 1;
	
	if(stat == 3){
		stat = 0; //round robin the display
	}

	EIFR = (1 << INTF1);
	return;
}


ISR(USART0_RX_vect) //ISR that fires when Xbee receives data
{
	ReceivedByte = UDR0; // Fetch the received byte value into the variable "ByteReceived"
	
	if(ReceivedByte == '1'){ //Update health
		player[0] -= 30;
		if(player[0] < 0){
			player[0] = 0;
		}
		return;
	}else if(ReceivedByte == '2'){ //Update Kills
		player[2] += 1;
		return;
	}else if(ReceivedByte == '3'){//Update Deaths
		player[3] += 1;
		return;
	}else if(ReceivedByte == '4'){//Update Team 1 score
		player[4] += 50;
		return;
	}else if(ReceivedByte == '5'){//Update Team 2 score
		player[5] += 50;
		return;
	}else if(ReceivedByte == '6'){//Respawn signal
		_delay_ms(50000);
		player[0] = 100;
		return;
	}

	return;
}

int main (void)
{
	_delay_ms(50); //Delay to allow LCD to turn on before initializing it
	//Initialize everything
	lcd_init();
	interrupt_init();
	USART_init();
	//int freq = 200;

	DDRA = 0b00001100; //set direction of PORTA, A0 and A1 are inputs, A2 and A3 are an outputs(LED, Laser)
	PORTA = 0b00001000; //Turn on status LED to signify system is on
	
	sei();                    // turn on interrupts

	
	lcd_command(0x01); //Clear Screen, Cursor Home
	
	while(1) {
		
		showInfo();		

	}
	
	return 0;
}
