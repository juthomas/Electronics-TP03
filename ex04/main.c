#ifndef __AVR_ATmega328P__
# define __AVR_ATmega328P__
# define F_CPU 16000000
#endif
#include <avr/interrupt.h>
#include <avr/io.h>
#define CPU_CLOCK 2000000 // 16Mhz -> / 8 2Mhz
#define SERIAL_8N1 0x06
#define NULL 0

#define ISR(vector, ...)            \
    void vector (void) __attribute__ ((signal,__INTR_ATTRS)) __VA_ARGS__; \
    void vector (void)

typedef struct	s_login
{
	const char *username;
	const char *password;
	const char *particules;
}				t_login;

#define LOG_NUMBER 3

static const t_login g_logs[LOG_NUMBER] = {
	(t_login){.username = "juthomas", .password = "juju", .particules = "le grand"},
	(t_login){.username = "jgourdin", .password = "jojo", .particules = "le magnifique"},
	(t_login){.username = "Party-Boi", .password = "jojo", .particules = "la star de reddit"}
};

void	wait_x_cpu_clocks(int32_t cpu_clocks)
{
	while (cpu_clocks > 0)
	{
		cpu_clocks-=3;
	}
}

void	custom_delay(uint32_t milli)
{
	//milli = 0,001s
	milli = milli *	2000;
	wait_x_cpu_clocks(milli - 5);
}

void blink_led(int normaly_on)
{
	DDRB |= (1 << PIND0);
	DDRB |= (1 << PIND1);
	PORTB = normaly_on;

	for (int i = 0; i < 10; i++)
	{
		PORTB ^= (1 << PIND0);
		PORTB ^= (1 << PIND1);
		custom_delay(100);
	}
	PORTB = 0b00000000;
}

void blink_success()
{
	blink_led(1 << PIND1);
}

void blink_failure()
{
	blink_led(NULL);
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
	// UCSR0C |= (1<<UCSZ00 | (1 << UCSZ01));
	//Enable Transmition and Reception and Interruption on Reception
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);// | (1 << RXCIE0);

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

int str_comp(char *s1, char *s2)
{
	while (*s1 == *s2 && *s1 && *s2)
	{
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}


void get_string_uart(int print_char, char str[50])
{
	char c = '\0';
	int i = 0;
	while (c != '\n')
	{
		c = uart_rx();

		if (c == 127)
		{
			if (i > 0)
			{
				uart_printstr("\033[1D\033[K");
				i--;
			}
		}
		else if (c == 13)
		{
			uart_printstr("\r\n");
			break;
		}
		else if ((c >= 32  && c < 126))
		{
			if (print_char == 1)
			{
				uart_tx(c);
			}
			else
			{
				uart_tx('*');
			}
			str[i] = c;
			i++;
		}
	}
	str[i] = '\0';
}

int main()
{
	uart_init(115200, SERIAL_8N1);

	char tmp_username[50];
	char tmp_password[50];
	for(;;)
	{
		uart_printstr("\033[1;36mBonjour ! Entrez votre login : \r\n");
		uart_printstr("\033[1;34musername: \033[1;35m");
		get_string_uart(1, tmp_username);
		uart_printstr("\033[1;34mpassword: \033[1;35m");
		get_string_uart(0, tmp_password);
		uart_printstr("\033[1;36m\r\n");
		int current_log = 0;
		int success = 0;
		while (current_log < LOG_NUMBER)
		{
			if ((str_comp(tmp_username, g_logs[current_log].username) == 0)
				&& (str_comp(tmp_password, g_logs[current_log].password) == 0))
			{
				success = current_log + 1;
			}
			current_log++;
		}

		if (success >= 1)
		{
			uart_printstr("\033[1;32mCC chef ");
			uart_printstr(g_logs[success-1].username);
			uart_printstr(" ");
			uart_printstr(g_logs[success-1].particules);
			uart_printstr("\033[1;36m\r\n");
			blink_success();
		}
		else
		{
			uart_printstr("\033[1;31mPtdr t ki?\033[1;37m\r\n");
			blink_failure();
		}
		uart_printstr("\r\n");
	}
}
