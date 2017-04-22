/*
 * Generic USART routines.
 *
 * License: MIT
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#include "usart.h"

#include <stdio.h>
#include <ctype.h>
#include <avr/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>


static unsigned char usart_echo = 1;


void usart_init(uint16_t ubrr, uint8_t x2)
{
	/* disable receiver and transmitter */
	UCSR0B = 0;

	/* set baud rate */
	UBRR0H = (ubrr >> 8);
	UBRR0L = ubrr;
	UCSR0A = x2 ? UCSR0A | (1 << U2X0) : UCSR0A & ~(1 << U2X0);

	/* enable receiver and transmitter */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	/* set frame format: 8N1 */
	UCSR0C = (0x3 << UCSZ00);
}

void usart_enable(void)
{
	/* enable receiver and transmitter */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

void usart_disable(void)
{
	/* disable receiver and transmitter */
	UCSR0B = 0;
}

void usart_enable_interrupts(uint8_t rx, uint8_t tx, uint8_t empty)
{
	UCSR0B |=
	    (rx ? (1 << RXCIE0) : 0) |
	    (tx ? (1 << TXCIE0) : 0) |
	    (empty ? (1 << UDRIE0) : 0);
}

void _usart_send_byte(uint8_t byte)
{
	/* Wait for empty transmit buffer. */
	while (!(UCSR0A & (1 << UDRE0)));

	/* Transmit data. */
	UDR0 = byte;
}

uint8_t usart_recv_byte(void)
{
	/* Wait for data to be received. */
	while (!(UCSR0A & (1 << RXC0)));

	/* Receive data. */
	return UDR0;
}

void usart_print(char *string)
{
	/* transmit until null */
	for ( ; (*string) != '\0'; string++)
	{
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = *string;
	}
}

void usart_print_line(char *string)
{
	/* transmit until null */
	for ( ; (*string) != '\0'; string++)
	{
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = *string;
	}
	usart_send_byte(0x0d);
	usart_send_byte(0x0a);
}

void usart_crlf(void)
{
	usart_send_byte(0x0d);
	usart_send_byte(0x0a);
}

void usart_echo_off(void)
{
	usart_echo = 0;
}

void usart_echo_on(void)
{
	usart_echo = 1;
}

void usart_enable_stdout(void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;
}

void usart_enable_stdin(void)
{
	static FILE usart_stdin = FDEV_SETUP_STREAM(NULL, usart_getchar, _FDEV_SETUP_READ);
	stdin = &usart_stdin;
}

void usart_receive_line(char *line, int len)
{
	int i = 0;
	unsigned char x;

	while (1)
	{
		/* Recieve. */
		x = usart_recv_byte();

		/* If carriage return, then line feed and break out. */
		if (x == 0x0d)
		{
			/* Transmit CR-LF. */
			if (usart_echo)
			{
				usart_send_byte(0x0d);
				usart_send_byte(0x0a);
			}

			/* End line. */
			line[i] = '\0';

			/* Break out. */
			break;
		}

		/* If del. */
		if (x == 0x7f)
		{
			/* If counter zero. */
			if (i < 1)
			{
				continue;
			}

			/* Decrease counter. */
			i--;

			/* Transmit backspace-space-backspace. */
			if (usart_echo)
			{
				usart_send_byte(0x08);
				usart_send_byte(0x20);
				usart_send_byte(0x08);
			}

			/* Continue. */
			continue;
		}

		/* If not printable char. */
		if (!isprint(x))
		{
			continue;
		}

		if (i < (len - 1))
		{
			/* Save char. */
			line[i] = x;

			/* Increase counter. */
			i++;

			/* Echo. */
			if (usart_echo)
			{
				usart_send_byte(x);
			}
		}
	}
}

int usart_putchar(char c, FILE *stream)
{
	usart_send_byte(c);
	return 0;
}

int usart_getchar(FILE *stream)
{
	return (int)usart_recv_byte();
}

