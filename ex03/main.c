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
	//UCSR0C = config;
	UCSR0C |= (1<<UCSZ00 | (1 << UCSZ01));
	//Enable Transmition and reception
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

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

char uart_rx(void)
{
	//(20.7)
	/* Wait for data to be received */
	while (!(UCSR0A & (1 << RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
}

ISR(USART_RX_vect) {
    uart_tx(uart_rx());;
}

int main()
{
	uart_init(115200, SERIAL_8N1);
	
	//DDRC |= (1 << PC5);
	sei();

	for(;;)
	{
		
	}
}
