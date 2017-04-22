/*
 * Generic Scope
 *
 * License: GNU/GPL, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi>
 */

#include "rs232.h"


/******************************************************************************/
struct rs232_buffer
{
	char *data;
	int size;
	int current;
	int is_full;
	struct rs232_buffer *next;
};

struct rs232_port
{
	int baudrate;
	char name[128];
	thread_t thread;
	int run;
	mutex_t lock;

	struct rs232_buffer *buffer;
	struct rs232_buffer *buffer_last;
	int buffer_size;
	int buffers_max;  
	char *buffer_break;
	int buffer_count;

#ifdef _WIN32
	HANDLE fd;
#else
	int fd;
#endif
};

struct rs232_port rs232_ports[RS232_MAX_OPEN_PORTS];
char **rs232_found_ports = NULL;


/******************************************************************************/
int rs232_run = 1;


/******************************************************************************/
void _rs232_fill_buffer(struct rs232_port *port, char *data, int size)
{
	int i;
	struct rs232_buffer *buffer = NULL;

	for (i = 0; i < size; i++)
	{
		/* allocate buffer if totally empty */
		if (port->buffer == NULL)
		{
			port->buffer = (struct rs232_buffer *)malloc(sizeof(*port->buffer));
			memset(port->buffer, 0, sizeof(*port->buffer));
			port->buffer->data = (char *)malloc(port->buffer_size);
			memset(port->buffer->data, 0, port->buffer_size);
			port->buffer->size = port->buffer_size;

			port->buffer_last = port->buffer;
			port->buffer_count = 1;
		}

		buffer = port->buffer_last;

		/* allocate new buffer if old one is full */
		if (buffer->is_full)
		{
			buffer = (struct rs232_buffer *)malloc(sizeof(*buffer));
			memset(buffer, 0, sizeof(*buffer));
			buffer->data = (char *)malloc(port->buffer_size);
			memset(buffer->data, 0, port->buffer_size);
			buffer->size = port->buffer_size;

			port->buffer_last->next = buffer;
			port->buffer_last = buffer;
			port->buffer_count++;
		}

		/* add data to current buffer (until break is found) */
		buffer->data[buffer->current] = data[i];
		buffer->current++;
		/* if buffer is full */
		if ((buffer->current + 1) >= buffer->size)
		{
			buffer->is_full = 1;
			buffer->size = buffer->current;
			buffer->current = 0;
		}
		/* check for breaking character(s) */
		else if (port->buffer_break != NULL)
		{
			if (strchr(port->buffer_break, (int)data[i]))
			{
				buffer->is_full = 1;
				buffer->size = buffer->current;
				buffer->current = 0;
			}
		}
	}
}


/******************************************************************************/
int _rs232_reader(void *data)
{
	struct rs232_port *port = (struct rs232_port *)data;
	char *buf;

	while (rs232_run && port->run)
	{
		int count = 0;
		char data[257];
		memset(data, 0, sizeof(data));

#ifdef _WIN32
		ReadFile(port->fd, data, sizeof(data) - 1, &count, NULL);
#else
		count = read(port->fd, data, sizeof(data) - 1);
#endif

		if (count > 0)
		{
			mutex_lock(&port->lock);
			_rs232_fill_buffer(port, data, (int)count);
			mutex_unlock(&port->lock);
		}

		while (port->buffer_count >= port->buffers_max)
		{
			printf("maximum buffers reached\n");
			buf = rs232_get_data(0, NULL);
			if (buf)
			{
				free(buf);
			}
		}
	}

	port->run = 0;

	return 0;
}


/******************************************************************************/
int rs232_init()
{
	memset(rs232_ports, 0, sizeof(rs232_ports));
	return 0;
}


/******************************************************************************/
void rs232_quit()
{
	int i;
	for (i = 0; i < RS232_MAX_OPEN_PORTS; i++)
	{
		if (rs232_ports[i].baudrate != 0)
		{
			rs232_close(i);
		}
	}
	memset(rs232_ports, 0, sizeof(rs232_ports));
}


/******************************************************************************/
int rs232_open(char *portname, int baudrate)
{
	int i;
	int port_id = -1;
	struct rs232_port *port = NULL;
#ifdef _WIN32
	DCB dcb;
#endif

	/* portname has to be shorter that 128 characters */
	if (strlen(portname) >= 128)
	{
		return -1;
	}

	/* find empty port id */
	for (i = 0; i < RS232_MAX_OPEN_PORTS; i++)
	{
		if (rs232_ports[i].baudrate == 0)
		{
			port_id = i;
			break;
		}
	}
	if (port_id < 0)
	{
		return -1;
	}

	/* generic port setup */
	port = &rs232_ports[port_id];
	memset(port, 0, sizeof(*port));
	port->baudrate = baudrate;
	strcpy(port->name, portname);
	port->buffer_size = RS232_DEFAULT_MAX_BUFFER_SIZE;
	port->buffers_max = RS232_DEFAULT_MAX_BUFFERS;
	mutex_init(&port->lock);

#ifdef _WIN32
	/* on windows, setup port */
	port->fd = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (port->fd == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	FillMemory(&dcb, sizeof(dcb), 0);
	if (!GetCommState(port->fd, &dcb))
	{
		return -1;
	}

	/* update DCB */
	dcb.BaudRate = baudrate;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	/* set new state */
	if (!SetCommState(port->fd, &dcb))
	{
		return -1;
	}
#else
	struct termios tio;

	/*
	 * Open device for reading and writing and not as controlling tty
	 * because we don't want to get killed if linenoise sends CTRL-C.
	 */
	int fd = open(portname, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		perror(portname);
		return -1;
	}

	memset(&tio, 0, sizeof(tio)); /* clear struct for new port settings */

	/*
	 * BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	 * CRTSCTS : output hardware flow control (only used if the cable has
	 *           all necessary lines. See sect. 7 of Serial-HOWTO)
	 * CS8     : 8N1 (8bit,no parity,1 stopbit)
	 * CLOCAL  : local connection, no contol
	 * CREAD   : enable receiving characters
	 */
	switch (baudrate)
	{
	case 38400:
		baudrate = B38400;
		break;
	case 57600:
		baudrate = B57600;
		break;
	case 115200:
		baudrate = B115200;
		break;
// TODO: http://stackoverflow.com/questions/4968529/how-to-set-baud-rate-to-307200-on-linux
		/*    case 250000:
		        {
		            struct serial_struct serinfo;
		            ioctl(fd, TIOCGSERIAL, &serinfo);
		            serinfo.flags = (serinfo.flags & ~ASYNC_SPD_MASK) | ASYNC_SPD_CUST;
		            serinfo.custom_divisor = (serinfo.baud_base + (baudrate / 2)) / baudrate;
		            int closest = serinfo.baud_base / serinfo.custom_divisor;

		            printf("moi\n");

		            if (closest < (baudrate * 98 / 100) || closest > (baudrate * 102 / 100)) {
		                fprintf(stderr, "cannot set serial port baudrate to %d, closest possible is %d\n", baudrate, closest);
		            }

		            printf("set port speed special\n");
		            //ioctl(fd, TIOCSSERIAL, &serinfo);
		            return -1;
		            //baudrate = B150000;
		            break;
		        }*/
	}

	tio.c_cflag = baudrate | CRTSCTS | CS8 | CLOCAL | CREAD;

	/*
	 * IGNPAR  : ignore bytes with parity errors
	 * ICRNL   : map CR to NL (otherwise a CR input on the other computer
	 *           will not terminate input)
	 * otherwise make device raw (no other input processing)
	 */
	tio.c_iflag = IGNPAR | ICRNL;

	/* raw output */
	tio.c_oflag = 0;

	/*
	 * disable all echo functionality, and don't send signals to calling program
	 */
	tio.c_lflag = 0;

	/*
	 * initialize all control characters
	 * default values can be found in /usr/include/termios.h, and are given
	 * in the comments, but we don't need them here
	 */
	tio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	tio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	tio.c_cc[VERASE]   = 0;     /* del */
	tio.c_cc[VKILL]    = 0;     /* @ */
	tio.c_cc[VEOF]     = 4;     /* Ctrl-d */
	tio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	tio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	tio.c_cc[VSWTC]    = 0;     /* '\0' */
	tio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	tio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	tio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	tio.c_cc[VEOL]     = 0;     /* '\0' */
	tio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	tio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	tio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	tio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	tio.c_cc[VEOL2]    = 0;     /* '\0' */

	/* now clean the modem line and activate the settings for the port */
	if (tcflush(fd, TCIFLUSH) < 0)
	{
		perror(portname);
	}
	if (tcsetattr(fd, TCSANOW, &tio) < 0)
	{
		perror(portname);
	}
	port->fd = fd;
#endif

	return 0;
}


/******************************************************************************/
int rs232_close(int id)
{
	struct rs232_port *port;
	struct rs232_buffer *buffer, *temp;

	if (id < 0 || id >= RS232_MAX_OPEN_PORTS)
	{
		return 0;
	}
	port = &rs232_ports[id];
	if (port->baudrate <= 0)
	{
		return 0;
	}

	port->run = 0;
	thread_wait(port->thread);
	mutex_destroy(&port->lock);

#ifdef _WIN32
	CloseHandle(port->fd);
#else
	close(port->fd);
#endif

	for (buffer = port->buffer; buffer; buffer = temp)
	{
		temp = buffer->next;
		free(buffer->data);
		free(buffer);
	}

	memset(port, 0, sizeof(*port));

	return 0;
}


/******************************************************************************/
char **rs232_find_ports(int *count)
{
	int i, found = 0;

	if (!rs232_found_ports)
	{
		rs232_found_ports = (char **)malloc(sizeof(char *) * 64);
		for (i = 0; i < 64; i++)
		{
			rs232_found_ports[i] = (char *)malloc(32);
		}
	}
	for (i = 0; i < 64; i++)
	{
		memset(rs232_found_ports[i], 0, 32);
	}

#ifdef _WIN32
	for (i = 0; i < 16; i++)
	{
		char portname[16];
		HANDLE fd;
		sprintf(portname, "COM%d", i);
		fd = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (fd == INVALID_HANDLE_VALUE)
		{
			continue;
		}
		CloseHandle(fd);
		strcpy(rs232_found_ports[found], portname);
		found++;
	}

	if (count)
	{
		*count = found;
	}
	return rs232_found_ports;

#else
	struct dirent *entry;
	DIR *dir = opendir("/dev");
	if (!dir)
	{
		return NULL;
	}

	found = 0;
	while ((entry = readdir(dir)))
	{
		int fd;
		struct serial_struct serinfo;
		char path[256];

		if (entry->d_type != DT_CHR)
		{
			continue;
		}
		if (strncmp(entry->d_name, "tty", 3) != 0)
		{
			continue;
		}

		sprintf(path, "/dev/%s", entry->d_name);
		fd = open(path, O_RDWR | O_NONBLOCK | O_NOCTTY);
		if (fd < 0)
		{
			continue;
		}
		if (ioctl(fd, TIOCGSERIAL, &serinfo) < 0)
		{
			continue;
		}
		if (serinfo.type == PORT_UNKNOWN)
		{
			continue;
		}

		strcpy(rs232_found_ports[found], path);
		found++;
		close(fd);
	}
	closedir(dir);

	if (count)
	{
		*count = found;
	}
	return rs232_found_ports;
#endif

	return NULL;
}


/******************************************************************************/
void rs232_set_break(int id, char *breakers)
{
	struct rs232_port *port = &rs232_ports[id];
	port->buffer_break = strdup(breakers);
}


/******************************************************************************/
void rs232_set_buffer_size(int id, int buffer_size)
{
	struct rs232_port *port = &rs232_ports[id];
	port->buffer_size = buffer_size;
}


/******************************************************************************/
void rs232_set_max_buffers(int id, int buffers_max)
{
	struct rs232_port *port = &rs232_ports[id];
	port->buffers_max = buffers_max;
}


/******************************************************************************/
int rs232_start_port(int id)
{
	struct rs232_port *port = &rs232_ports[id];

	port->run = 1;
	port->thread = thread_create(_rs232_reader, (void *)port);

	return 0;
}


/******************************************************************************/
char *rs232_get_data(int id, int *size)
{
	struct rs232_port *port;
	struct rs232_buffer *buffer;
	char *data = NULL;

	if (id < 0 || id >= RS232_MAX_OPEN_PORTS)
	{
		return NULL;
	}
	port = &rs232_ports[id];
	if (port->baudrate <= 0)
	{
		return NULL;
	}

	mutex_lock(&port->lock);
	buffer = port->buffer;

	if (!buffer || port->buffer_count < 1)
	{
		/* if no buffer or buffer count invalid */
	}
	else if (!buffer->is_full)
	{
		/* if current buffer is not full yet */
	}
	else     /* come here if there is a full buffer available */
	{
		if (size)
		{
			*size = buffer->size;
		}

		data = buffer->data;
		port->buffer = port->buffer->next;
		free(buffer);
		port->buffer_count--;
	}

	mutex_unlock(&port->lock);

	return data;
}

