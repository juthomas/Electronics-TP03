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

typedef struct	s_login
{
	const char *username;
	const char *password;
	const char *particules;
}				t_login;

#define LOG_NUMBER 2

const t_login g_logs[LOG_NUMBER] = {
	(t_login){.username = "juthomas", .password = "juju", .particules = "le grand"},
	(t_login){.username = "jgourdin", .password = "jojo", .particules = "le magnifique"}
};


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

char username[50] = "juthomas";
char password[50] = "juju";

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
	//Enable interrupts
	// SREG|=(1<<7);
	char tmp_username[50];
	char tmp_password[50];
	for(;;)
	{
		uart_printstr("Bonjour ! Entrez votre login : \r\n");
		uart_printstr("username: ");
		get_string_uart(1, tmp_username);
		uart_printstr("password: ");
		get_string_uart(0, tmp_password);
		uart_printstr("\r\n");
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
			uart_printstr("CC chef ");
			uart_printstr(g_logs[success-1].username);
			uart_printstr(" ");
			uart_printstr(g_logs[success-1].particules);
			uart_printstr("\r\n");
		}
		else
		{
			uart_printstr("Ptdr t ki?\r\n");
		}
		uart_printstr("\r\n");
	}
}
