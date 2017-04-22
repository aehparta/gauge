
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <usart.h>


/* eeprom addresses */
#define EEA_SWITCHES            (void *)0
#define EEA_SLIDER1             (void *)4
#define EEA_SLIDER2             (void *)8
#define EEA_DEVICE_NAME         (void *)64

/* device name is saved here */
static char device_name[32] = "Light Switch";

/* switch(es) */
static volatile uint8_t switches = 1;

/* slider */
static volatile uint8_t slider1 = 0x64;
static volatile uint8_t slider2 = 0x64;


ISR(USART_RX_vect)
{
	static char line[64];
	static int line_c = 0;

	uint8_t c = UDR0;

	if (c == '\n' || c == '\r' || line_c >= (sizeof(line) - 1))
	{
		if (line_c <= 0)
		{
			return;
		}
		line[line_c] = '\0';

		if (line_c == 2 && line[0] == 'C')
		{
			switches = (line[1] == '0' ? 0 : 1);
			eeprom_write_byte(EEA_SWITCHES, switches);
			OCR0A = switches ? slider1 << 1 : 0x00;
			OCR0B = switches ? slider2 << 1 : 0x00;
		}
		else if (line_c > 1 && line[0] == 'D')
		{
			slider1 = (uint8_t)strtol(line + 1, NULL, 16);
			if (slider1 > 0x64)
			{
				slider1 = 0x64;
			}
			eeprom_write_byte(EEA_SLIDER1, slider1);
			OCR0A = switches ? slider1 << 1 : 0x00;
		}
		else if (line_c > 1 && line[0] == 'E')
		{
			slider2 = (uint8_t)strtol(line + 1, NULL, 16);
			if (slider2 > 0x64)
			{
				slider2 = 0x64;
			}
			eeprom_write_byte(EEA_SLIDER2, slider2);
			OCR0B = switches ? slider2 << 1 : 0x00;
		}
		else if (strcmp(line, "get:config") == 0)
		{
			printf("\n"); /* clear last send just to be sure */
			printf("config:channel:C:name:%s\n", device_name);
			printf("config:channel:C:type:switch\n");
			printf("config:channel:C:mode:sink\n");
			printf("config:channel:C:method:push\n");
			printf("config:channel:D:static:name:Slider 1\n");
			printf("config:channel:D:type:slider\n");
			printf("config:channel:D:mode:sink\n");
			printf("config:channel:D:method:push\n");
			printf("config:channel:D:max:64\n");
			printf("config:channel:E:static:name:Slider 2\n");
			printf("config:channel:E:type:slider\n");
			printf("config:channel:E:mode:sink\n");
			printf("config:channel:E:method:push\n");
			printf("config:channel:E:max:64\n");
			printf("C%d\n", switches ? 1 : 0);
			printf("D%x\n", slider1);
			printf("E%x\n", slider2);
		}
		else if (strncmp(line, "set:channel:C:name:", 19) == 0 && line_c > 19)
		{
			strncpy(device_name, line + 19, sizeof(device_name));
			eeprom_write_block(device_name, EEA_DEVICE_NAME, sizeof(device_name));
		}

		line_c = 0;
	}
	else
	{
		line[line_c] = c;
		line_c++;
	}
}

/*
 * Initialize timer(s)
 */
void timers_init(void)
{
	/* timer 0 mode to PWM */
	TCCR0A = 0b10100011;
	TCCR0B = 0b00000001;
	OCR0A = slider1 << 1;
	OCR0B = slider2 << 1;
}

int main(void)
{
	cli();

	/* defaults */
	DDRB = 0x00;
	DDRC = 0x00;
	DDRD = 0x60;
	PORTD = 0x00;

	/* initialize timing */
	timers_init();

	/* read saved values from eeprom */
	if (eeprom_read_byte(EEA_DEVICE_NAME) != 0xff)
	{
		eeprom_read_block(device_name, EEA_DEVICE_NAME, sizeof(device_name));
	}

	/* initialize usart and bluetooth */
	usart_enable_stdout();
	usart_init(12, 0); /* now set usart to 38400 */
	usart_enable_interrupts(1, 0, 0);

	/* program loop */
	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();
	while (1)
	{
		sleep_mode();
	}

	return 0;
}
