
#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "gpio.h"
#include "usart.h"
#include "nvic.h"
#include "linkedlist.h"

/* internal queue struct */
struct usart_queue_data
{
	char *data;
	int total;
	int current;
	void *prev;
	void *next;
};
struct usart_queue_data *usart_queue_first = NULL;
struct usart_queue_data *usart_queue_last = NULL;

struct usart_queue_data *usart_recv_current = NULL;
struct usart_queue_data *usart_recv_queue_first = NULL;
struct usart_queue_data *usart_recv_queue_last = NULL;


void usart_qprintf(uint32_t usart, const char *format, ...)
{
	va_list args;
	struct usart_queue_data *data;

	/* allocate queue item */
	data = malloc(sizeof(*data));
	if (!data)
	{
		return;
	}
	data->current = 0;

	/* print */
	va_start(args, format);
	data->total = vasprintf(&data->data, format, args);
	va_end(args);
	if (data->total < 0)
	{
		free(data);
		return;
	}

	/* @todo atomic append to list */
	// disable_interrupts();
	LL_APP(usart_queue_first, usart_queue_last, data);
	// enable_interrupts();

	/* enable usart transmit interrupt */
	io32_t CR1 = P_REG(usart, USART_CR1_offs);
	*CR1 |= USART_CR1_TXEIE;
}

char *usart_qget(void)
{
	struct usart_queue_data *data;

	/* atomic read from list */
	disable_interrupts();
	LL_GET(usart_recv_queue_first, usart_recv_queue_last, data);
	enable_interrupts();

	if (data)
	{
		char *line = data->data;
		free(data);
		return line;
	}

	return NULL;
}

void usart_init(uint32_t usart, uint32_t baudrate)
{
	gpio_set_af(GPIOA, BIT2 | BIT3, AF7);

	/* USART2 at A2 (tx), A3 (rx) */
	RCC_APB1ENR |= RCC_APB1ENR_USART2EN;

	/* calculate and set baud rate configuration */
	double mantissa;
	double fraction = modf(F_CPU / ((double)baudrate * 16), &mantissa);
	USART2_BRR = (((uint32_t)mantissa) << 4) | ((uint32_t)round(fraction * 16));

	/* USART enable */
	USART2_CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;

	/* enable interrupt on data receive */
	nvic_enable_irq(NVIC_USART2_IRQ);
	USART2_CR1 |= USART_CR1_RXNEIE;
}

void usart_handle_interrupts(uint32_t usart)
{
	io32_t SR = P_REG(usart, USART_SR_offs);
	uint32_t sr = *SR;

	if (sr & USART_SR_ORE)
	{
		/* RXIE enables also ORE - must handle ORE */
		*SR &= ~USART_SR_ORE;
	}
	else if (sr & USART_SR_RXNE)
	{
		/* receive data byte */
		io32_t DR = P_REG(usart, USART_DR_offs);
		uint8_t b = *DR & 0xFF;

		if (b == '\r' || b == '\n')
		{
			/* this is \r or \n so we are finished with this line */
			if (usart_recv_current)
			{
				usart_recv_current->data[usart_recv_current->current] = '\0';
				LL_APP(usart_recv_queue_first, usart_recv_queue_last, usart_recv_current);
				usart_recv_current = NULL;
			}
		}
		else
		{
			/* append to receive queue */
			if (!usart_recv_current)
			{
				usart_recv_current = malloc(sizeof(*usart_recv_current));
				memset(usart_recv_current, 0, sizeof(*usart_recv_current));
			}
			if (usart_recv_current->total <= (usart_recv_current->current + 1))
			{
				usart_recv_current->total += 16;
				usart_recv_current->data = realloc(usart_recv_current->data, usart_recv_current->total);
			}
			usart_recv_current->data[usart_recv_current->current] = b;
			usart_recv_current->current++;
		}
	}
	else if (sr & USART_SR_TXE)
	{
		/* transmit can be done */
		io32_t DR = P_REG(usart, USART_DR_offs);
		*DR = usart_queue_first->data[usart_queue_first->current];

		usart_queue_first->current++;
		if (usart_queue_first->current >= usart_queue_first->total)
		{
			struct usart_queue_data *data;
			LL_GET(usart_queue_first, usart_queue_last, data);
			free(data->data);
			free(data);
			if (!usart_queue_first)
			{
				/* no more data, disabled transmit interrupt */
				io32_t CR1 = P_REG(usart, USART_CR1_offs);
				*CR1 &= ~USART_CR1_TXEIE;
			}
		}
	}
}

bool usart_wait_rx(uint32_t usart)
{
	for (uint32_t i = 0; i < USART_RX_TIMEOUT; i++)
	{
		if (usart_rx_ready(usart))
		{
			return true;
		}
	}
	return false;
}

bool usart_wait_tx(uint32_t usart)
{
	for (uint32_t i = 0; i < USART_TX_TIMEOUT; i++)
	{
		if (usart_tx_ready(usart))
		{
			return true;
		}
	}
	return false;
}

bool usart_rx_ready(uint32_t usart)
{
	io32_t SR = P_REG(usart, USART_SR_offs);

	return (*SR & USART_SR_RXNE);
}

bool usart_tx_ready(uint32_t usart)
{
	io32_t SR = P_REG(usart, USART_SR_offs);

	return (*SR & USART_SR_TXE);
}

char usart_rx_char(uint32_t usart)
{
	io32_t DR = P_REG(usart, USART_DR_offs);

	return *DR & 0xFF;
}

uint32_t usart_rx_string(uint32_t usart, char *buffer, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i++)
	{
		if (!usart_wait_rx(usart))
		{
			break;
		}
		buffer[i] = usart_rx_char(usart);
	}

	buffer[i] = 0;
	return i;
}

void usart_tx_char(uint32_t usart, uint8_t c)
{
	io32_t DR = P_REG(usart, USART_DR_offs);
	usart_wait_tx(usart);
	*DR = c;
}

void usart_tx_string(uint32_t usart, const char *string)
{
	char c;
	while ((c = *string++) != 0)
	{
		usart_tx_char(usart, c);
	}
}

