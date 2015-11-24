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

typedef struct{
	int health = 100;
	int ammo = 12;
	int kills = 0;
	int deaths = 0;
	int lives = 5;
	int score = 100;
	};

int player[6] = {100,30,0,0,5,100}; //Array holding player HP, ammo, kills, deaths, lives remaining, and team score -- respectively
int stat = 0;
int times;

void lcd_init()
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

void writeToLCD(char* str)	//prints a string to the LCD through LCD_CHAR
{
	do
	{
		char toLCD = *str;
		str++;
		lcd_char(toLCD);
	} while(*(str) != '\0');
}

void lcd_command(char cmd)
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

void lcd_char(char data)
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

int scale(int mag){ //determines magnitude of the number so it can be sent to the LCD correctly
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
	}
}

void healthAmmo(int health, int ammo){ //Sends data to the LCD
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
	if(scale1 == 1){
		lcd_char('0');
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
		lcd_char('0');
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
	lcd_char('3');
	lcd_char('0');
}

void showInfo(){
	lcd_command(0x02); // Cursor Home
	_delay_ms(5);
	healthAmmo(player[0], player[1]);
}

void calcPeriod(int fq){

	

}


ISR (INT0_vect) //ISR for shot fired
{
		if(player[1] <= 0){ //reload
			_delay_ms(20000);
			player[1] = 30;
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
	stat++;
	
	if(stat > 4){
		stat = 0; //round robin the display
	}

	EIFR = (1 << INTF1);
}

void interrupt_init(){
	  EICRA = 0xF;				// set INT0 and INT1 to trigger on rising edge
	  DDRD = 0; //All pins on PORTD are inputs

	  PORTD = 0xC;

	  EIMSK = 0x3;

	  sei();                    // turn on interrupts
}

int main (void)
{
	lcd_init();
	interrupt_init();
	int freq = 200;

	DDRA = 0b00001100; //set direction of PORTA, A0 and A1 are inputs, A2 and A3 are an outputs(LED, Laser)
	PORTA = 0b00001000; //Turn on status LED to signify system is on
	
	lcd_command(0x01); //Clear Screen, Cursor Home
	
	while(1) {
		
		_delay_ms(20);
		showInfo();
		_delay_ms(800);
		

	}
	
}
