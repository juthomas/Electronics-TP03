#ifndef __AVR_ATmega328P__
# define __AVR_ATmega328P__
# define F_CPU 16000000
#endif
#include <avr/interrupt.h>
#include <avr/io.h>
#define CPU_CLOCK 2000000 // 16Mhz -> / 8 2Mhz
#define SERIAL_8N1 0x06

#define ISR(vector, ...)            \
    void vector (void) __attribute__ ((signal,__INTR_ATTRS)) __VA_ARGS__; \
    void vector (void)


void	wait_x_cpu_clocks(int32_t cpu_clocks)
{
	while (cpu_clocks > 0)
	{
		cpu_clocks-=3;
	}
}

void	custom_delay(int32_t milli)
{
	//milli = 0,001s
	milli = milli *	2000;
	wait_x_cpu_clocks(milli - 5);
}


void uart_init(uint32_t baud, uint8_t config)
{
	//(20.5)
	uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;

	UCSR0A = 1 << U2X0;

	//Setting baudrate
	UBRR0H = baud_setting >> 8;
	UBRR0L = baud_setting;

	//Setting frame format config 
	UCSR0C = config;

	//Enable Transmition (RXEN0 for reception)
	UCSR0B = (1 << TXEN0);

}

void uart_tx(char c)
{
	//Wait for empty buffer (20.6.1)
	while (!( UCSR0A & (1<<UDRE0)));
	//Put char into register buffer
	UDR0 = c;
}

void uart_printstr(const char* str)
{
	while (*str)
	{
		uart_tx(*str++);
	}
}

ISR(TIMER1_COMPA_vect)
{
	uart_printstr("Hello World!\r\n");
}


int main()
{
	uart_init(115200, SERIAL_8N1);

	//Timer on 16Mhz/1024*2sec // A REVOIR parait comme 1.5sec
	// OCR1A = 62500;
	OCR1A = F_CPU / 1024 * 2 - 1;
	//Mode 4 (Table 16-4) et Prescaller sur 1024 (Table 16-5)
	TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
	//Toggle On compare match (Table 16-1)
	TCCR1A = 1 << COM1A0;
	//Timer/Counter1, Output Compare B Match Interrupt Enable (16.11.8)
	TIMSK1 |= OCIE1B; 
	//Enable interrupts
	SREG |= (1 << SREG_I);
	for(;;);
}
