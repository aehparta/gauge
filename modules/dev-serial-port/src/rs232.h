/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#ifndef __RS232_H__
#define __RS232_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#else
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#endif

#include "os.h"


/******************************************************************************/
#define RS232_MAX_OPEN_PORTS 16
#define RS232_DEFAULT_MAX_BUFFER_SIZE 512
#define RS232_DEFAULT_MAX_BUFFERS 512


/******************************************************************************/
/** Initialize RS232 library. */
int rs232_init();
/** Free resources and quit. */
void rs232_quit();
/**
 * Open RS232(COM)-port.
 */
int rs232_open(char *portname, int baudrate);
/**
 * Close RS232 port.
 */
int rs232_close(int id);
/**
 * Find available RS232 ports.
 */
char **rs232_find_ports(int *count);
/* Set breaker character(s). */
void rs232_set_break(int id, char *breakers);
/* Set maximum single buffer size (default RS232_DEFAULT_MAX_BUFFER_SIZE bytes). */
void rs232_set_buffer_size(int id, int buffer_size);
/* Set maximum amount of buffers (default RS232_DEFAULT_MAX_BUFFERS). */
void rs232_set_max_buffers(int id, int buffers_max);
/**
 * Start listening on given port.
 */
int rs232_start_port(int id);
/**
 * Receive data from buffered input.
 */
char *rs232_get_data(int id, int *size);


/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* __RS232_H__ */
