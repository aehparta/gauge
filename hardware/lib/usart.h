/*
 * Generic USART routines.
 *
 * License: MIT
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifndef __USART_H__
#define __USART_H__

#include <stdio.h>
#include <inttypes.h>


/**
 * Init USART.
 */
void usart_init(uint16_t ubrr, uint8_t x2);

/**
 * Enable USART.
 */
void usart_enable(void);

/**
 * Disable USART.
 */
void usart_disable(void);

/**
 * Enable USART interrupts.
 */
void usart_enable_interrupts(uint8_t rx, uint8_t tx, uint8_t empty);

/**
 * Transmit byte trough USART.
 */
// void usart_send_byte(uint8_t byte);
#define usart_send_byte(byte)   do { while (!(UCSR0A & (1 << UDRE0))); UDR0 = byte; } while(0)

/**
 * Recv byte from USART.
 */
uint8_t usart_recv_byte(void);

/**
 * Print given string trough USART.
 */
void usart_print(char *string);

/**
 * Print given line trough USART.
 */
void usart_print_line(char *line);

/**
 * Output carriage return and line feed to USART.
 */
void usart_crlf(void);

/**
 * Echo off.
 */
void usart_echo_off(void);

/**
 * Echo on.
 */
void usart_echo_on(void);

void usart_enable_stdout(void);

void usart_enable_stdin(void);

/**
 * Receive line from USART (Line end is CR).
 */
void usart_receive_line(char *, int);

int usart_putchar(char c, FILE *stream);

int usart_getchar(FILE *stream);


#endif /* __USART_H__ */
