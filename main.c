/*
 * serial.c
 * Created: 24/02/2020 10:01:10
 * Author : Robin Westerik
 */ 

#include <avr/io.h>

#define F_CPU 16000000
#define BAUD  9600
#define BRC   (F_CPU/16/BAUD)-1 //Baud rate formula
#define TX_BUFFER_SIZE 128 //Buffer size

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int mapper(float adcVal)
{
	return (adcVal/1024)*500; 
}

void send(int adcVal)
{
	int msb=adcVal/100; int mid=(adcVal-(msb*100))/10; int lsb=(adcVal-(mid*10)-(msb*100));
	//Show 1.75V
	push(msb); //msb
	_delay_ms(1);
	UDR0=44; // ascii for ,
	_delay_ms(1);
	push(mid); //mid
	push(lsb); //lsb	
	_delay_ms(1);
	UDR0=86; //V
	_delay_ms(1);
	UDR0=13; //ascii for Shift Out
	_delay_ms(1);
	UDR0=10; //ascii for Line Feed
	_delay_ms(60);
}

void push(int toPush)
{
	_delay_ms(1);
	UDR0=(toPush+48); //Read ADCH, offset it, and put it towards TX
}

int main(void)
{
	//Serial communication
	UBRR0H = (BRC >> 8); //The baud rate of UART/USART is set using the 16-bit wide UBRR register
	UBRR0L = BRC;        //
	
	UCSR0B = (1 << TXEN0); //Enable interrupt on receive and Enable transmitter
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); //Select 8 bits
	
	
	//ADC conversion
	PRR &= ~(1 << PRADC); //Set power reduction bit to low to enable ADC
	ADMUX |= (1 << REFS0) & ~(1 << REFS1); //Selects voltage reference to AVcc
	ADMUX &= ~(1 << MUX0) & ~(1 << MUX1) & ~(1 << MUX2) & ~(1 << MUX3); //Select input channel ADC0 by writing MUX bits in ADMUX
	ADMUX |= (0 << ADLAR); //Set ADLAR bit to output left-adjusted
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); //Set prescaler with ADPS bits in ADCSRA (division factor is 128)
	ADCSRA |= (1 << ADEN); //Set ADEN in ADCSRA to start ADC clock (start ADC)
	ADCSRA |= (1 << ADSC); //Start first conversion (Set ADSC bit)
	
	while(1)
	{
		if (ADCSRA & ~(1 << ADSC)) //If ADSC bit is low, conversion has finished
		{
			send(mapper(ADCL | (ADCH<<8))); //Read ADCH, offset it, and put it towards TX
			ADCSRA |= (1 << ADSC); //Start conversion again (Set ADSC bit)
		}
	}
}

