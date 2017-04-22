
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <glcd/glcd.h>
#include <glcd/fonts/font5x7.h>
#include <glcd/fonts/Verdana_11x12_Hex.h>
#include <glcd/fonts/Verdana_19x18_Hex.h>
#include <i2c.h>
#include <usart.h>


/* eeprom addresses */
#define EEA_ADC_ZERO            (void *)0
#define EEA_ADC_DIV             (void *)4
#define EEA_SWITCHES            (void *)8
#define EEA_DEVICE_NAME         (void *)64
#define EEA_DEVICE_SW1_NAME     (void *)96
#define EEA_DEVICE_SW2_NAME     (void *)128
/* for adc */
#define ADC_DEFAULT_ZERO        2048.0;
#define ADC_DEFAULT_DIV         4.0;
/* adc sample buffer size, this MUST be a power of 2, maximum of 256 */
#define ADC_SAMPLES             128 /* 160 ms */

/* device name is saved here */
static char device_name[32] = "current probe";
static char device_sw1_name[32] = "switch 1";
static char device_sw2_name[32] = "switch 2";

/* sample buffer */
static uint16_t samples[ADC_SAMPLES];
static volatile uint16_t sample_c = 0;

/* zero and divider */
static volatile float adc_zero;
static volatile float adc_div;

/* current i2c (adc) state */
static volatile uint8_t i2c_state = 0;

/* switches */
static volatile uint8_t switches = 0;

/* slider */
static volatile uint8_t slider_red = 0;
static volatile uint8_t slider_green = 20;
static volatile uint8_t slider_counter = 0;

/*
 * Timer 0 interrupt, called at 1600 Hz.
 */
ISR(TIMER0_COMPA_vect)
{
	/* trigger read of adc sample: send i2c start */
	i2c_state = 0;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);

	/* slider pwm */
	PORTC = (PORTC & ~0x03) | (slider_red > slider_counter ? 0 : 1) | (slider_green > slider_counter ? 0 : 2);
	slider_counter = slider_counter > 99 ? 0 : slider_counter + 10;
}

/*
 * Handle ADC.
 */
ISR(TWI_vect)
{
	static uint16_t w;

	switch (i2c_state)
	{
	case 0:
		/* address + set read */
		TWDR = 0b10010101;
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
		break;
	case 1:
		/* set for reading higher byte + ack */
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
		break;
	case 2:
		/* read higher byte + ack */
		w = TWDR << 8;
		/* set for reading lower byte + nack */
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
		break;
	case 3:
		/* read lower byte + nack */
		w |= TWDR;
		samples[sample_c] = w;
		sample_c++;
		sample_c &= (ADC_SAMPLES - 1);
		/* stop */
		TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
		break;
	}

	i2c_state++;
}

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

		if (line_c == 2 && line[0] == 'B')
		{
			switches = (switches & ~0x01) | (line[1] == '0' ? 1 : 0);
			PORTC = (switches & 0x3) << 2;
			eeprom_write_byte(EEA_SWITCHES, switches);
		}
		else if (line_c == 2 && line[0] == 'C')
		{
			switches = (switches & ~0x02) | (line[1] == '0' ? 2 : 0);
			PORTC = (switches & 0x3) << 2;
			eeprom_write_byte(EEA_SWITCHES, switches);
		}
		else if (line_c > 1 && line[0] == 'D')
		{
			slider_red = (uint8_t)strtol(line + 1, NULL, 16);
		}
		else if (line_c > 1 && line[0] == 'E')
		{
			slider_green = (uint8_t)strtol(line + 1, NULL, 16);
		}
		else if (strcmp(line, "get:config") == 0)
		{
			printf("\n"); /* clear last send just to be sure */
			printf("config:channels:2\n");
			printf("config:channel:A:name:%s\n", device_name);
			printf("config:channel:A:type:current\n");
			printf("config:channel:A:mode:source\n");
			printf("config:channel:A:method:push\n");
			printf("config:channel:A:format:hex\n");
			printf("config:channel:A:interval:0.000625\n");
			printf("config:channel:A:max:200.0\n");
			printf("config:channel:A:min:-200.0\n");
			printf("config:channel:A:resolution:.1\n");
			printf("config:channel:A:zero:%f\n", adc_zero);
			printf("config:channel:A:divider:%f\n", adc_div);
			printf("config:channel:B:name:%s\n", device_sw1_name);
			printf("config:channel:B:type:switch\n");
			printf("config:channel:B:mode:sink\n");
			printf("config:channel:B:method:push\n");
			printf("config:channel:C:name:%s\n", device_sw2_name);
			printf("config:channel:C:type:switch\n");
			printf("config:channel:C:mode:sink\n");
			printf("config:channel:C:method:push\n");
			printf("config:channel:D:static:name:Slider Red\n");
			printf("config:channel:D:type:slider\n");
			printf("config:channel:D:mode:sink\n");
			printf("config:channel:D:method:push\n");
			printf("config:channel:E:static:name:Slider Green\n");
			printf("config:channel:E:type:slider\n");
			printf("config:channel:E:mode:sink\n");
			printf("config:channel:E:method:push\n");
			printf("B%d\n", (switches & 0x01) ? 0 : 1);
			printf("C%d\n", (switches & 0x02) ? 0 : 1);
			printf("D%x\n", slider_red);
			printf("E%x\n", slider_green);
		}
		else if (strncmp(line, "set:channel:A:name:", 19) == 0 && line_c > 19)
		{
			strcpy(device_name, line + 19);
			eeprom_write_block(device_name, EEA_DEVICE_NAME, sizeof(device_name));
		}
		else if (strncmp(line, "set:channel:B:name:", 19) == 0 && line_c > 19)
		{
			strcpy(device_name, line + 19);
			eeprom_write_block(device_name, EEA_DEVICE_SW1_NAME, sizeof(device_sw1_name));
		}
		else if (strncmp(line, "set:channel:C:name:", 19) == 0 && line_c > 19)
		{
			strcpy(device_name, line + 19);
			eeprom_write_block(device_name, EEA_DEVICE_SW2_NAME, sizeof(device_sw2_name));
		}

		line_c = 0;
	}
	else
	{
		line[line_c] = c;
		line_c++;
	}
}

ISR(USART_UDRE_vect)
{
}

/*
 * Initialize timer(s)
 */
void timers_init(void)
{
	/* timer 0 mode to CTC */
	TCCR0A = 0b00000010;
	/* divide by 256 -> 43.2 kHz */
	TCCR0B = 0b00000100;
	/* count to 26, 43.2 kHz / 27 = 1600 Hz */
	OCR0A = 26;
	/* compare match interrupt enable */
	TIMSK0 = 0b00000010;
}

void calibrate_zero(void)
{
	int i;
	int32_t sum = 0;

	cli();

	for (i = 0; i < ADC_SAMPLES; i++)
	{
		sum += samples[i];
	}
	adc_zero = (float)sum / ADC_SAMPLES;
	eeprom_write_float(EEA_ADC_ZERO, adc_zero);
	printf("config:channel:A:zero:%f\n", adc_zero);

	sei();
}

void calibrate_5A(void)
{
	int i;
	int32_t sum = 0;

	cli();

	for (i = 0; i < ADC_SAMPLES; i++)
	{
		sum += samples[i];
	}
	adc_div = (((float)sum / ADC_SAMPLES) - adc_zero) / 4.99;
	eeprom_write_float(EEA_ADC_DIV, adc_div);
	printf("config:channel:A:divider:%f\n", adc_div);

	sei();
}

void sample_send_16bit(uint8_t channel, uint16_t value)
{
	uint8_t b;
	usart_send_byte('A' + channel); /* channel number */
	b = value >> 12;
	usart_send_byte(b > 0x9 ? 'a' - 0xa + b : '0' + b);
	b = (value >> 8) & 0xf;
	usart_send_byte(b > 0x9 ? 'a' - 0xa + b : '0' + b);
	b = (value >> 4) & 0xf;
	usart_send_byte(b > 0x9 ? 'a' - 0xa + b : '0' + b);
	b = value & 0xf;
	usart_send_byte(b > 0x9 ? 'a' - 0xa + b : '0' + b);
	usart_send_byte('\n');
}

void sample_send_12bit(uint8_t channel, uint16_t value)
{
	uint8_t b;
	usart_send_byte('A' + channel); /* channel number */
	b = (value >> 8) & 0xf;
	usart_send_byte(b > 0x9 ? 'a' - 0xa + b : '0' + b);
	b = (value >> 4) & 0xf;
	usart_send_byte(b > 0x9 ? 'a' - 0xa + b : '0' + b);
	b = value & 0xf;
	usart_send_byte(b > 0x9 ? 'a' - 0xa + b : '0' + b);
	usart_send_byte('\n');
}

int main(void)
{
	int i;
	uint8_t calibrate_zero_flag = 0;
	uint8_t calibrate_5A_flag = 0;
	uint16_t sample_c_send = 0;
	uint16_t display_delay = 0;
	int32_t display_samples_squared = 0;

	cli();

	/* default all ports as inputs */
	DDRB = 0x00;
	DDRC = 0x00;
	DDRD = 0x00;

	/* PORTD has some outputs */
	DDRD = 0x1c; /* PD2 is power for the bluetooth module, PD3 and PD4 as debug outputs */
	PORTD = 0x04; /* power to the bluetooth! */

	/* read saved values from eeprom */
	adc_zero = eeprom_read_float(EEA_ADC_ZERO);
	adc_div = eeprom_read_float(EEA_ADC_DIV);
	if (eeprom_read_byte(EEA_DEVICE_NAME) != 0xff)
	{
		eeprom_read_block(device_name, EEA_DEVICE_NAME, sizeof(device_name));
	}
	switches = eeprom_read_byte(EEA_SWITCHES);
	if (switches == 0xff)
	{
		switches = 0;
	}
	if (eeprom_read_byte(EEA_DEVICE_SW1_NAME) != 0xff)
	{
		eeprom_read_block(device_sw1_name, EEA_DEVICE_SW1_NAME, sizeof(device_sw1_name));
	}
	if (eeprom_read_byte(EEA_DEVICE_SW2_NAME) != 0xff)
	{
		eeprom_read_block(device_sw2_name, EEA_DEVICE_SW2_NAME, sizeof(device_sw2_name));
	}

	/* initialize switches */
	DDRC = 0x0f;
	PORTC = (switches & 0x3) << 2;

	/* initialize usart and bluetooth */
	usart_enable_stdout();
	usart_init(2, 1); /* now set usart to 460800 too */
	usart_enable_interrupts(1, 0, 0);

	/* initialize adc (i2c) */
	for (i = 0; i < ADC_SAMPLES; i++)
	{
		samples[i] = (uint16_t)adc_zero;
	}
	/* scl to ~394.97kHz */
	TWSR = 0x00;
	TWBR = 0x06;
	TWCR = (1 << TWEN);

	/* initialize display */
	glcd_init();
	glcd_set_contrast(60);

	/* initialize timing */
	timers_init();

	/* program loop */
	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();
	while (1)
	{
		/* send new sample(s) */
		while (sample_c_send != sample_c)
		{
			uint16_t sample = samples[sample_c_send];
			sample_send_12bit(0, sample);
			sample_c_send++;
			sample_c_send &= (ADC_SAMPLES - 1);

			int32_t sample32 = (int32_t)sample - adc_zero;
			display_samples_squared += (sample32 * sample32);
			display_delay++;
		}

		/* calculate rms */
		if (display_delay >= 800)
		{
			char str[32];

			double rms = sqrt(display_samples_squared / display_delay) / adc_div;
			display_samples_squared = 0;
			display_delay = 0;

			/* check zero calibration pin */
			if (!(PIND & 0x80))
			{
				calibrate_zero_flag = 2;
			}
			else if (calibrate_zero_flag == 2)
			{
				calibrate_zero();
				calibrate_zero_flag = 0;
			}

			/* check 5A calibration pin */
			if (!(PIND & 0x40))
			{
				calibrate_5A_flag = 2;
			}
			else if (calibrate_5A_flag == 2)
			{
				calibrate_5A();
				calibrate_5A_flag = 0;
			}

			/* display result */
			glcd_clear_buffer();

			glcd_set_font(Verdana_19x18_Hex, 19, 18, 43, 70);
			sprintf(str, "%.1f", rms);
			glcd_draw_string_xy_align_right(GLCD_LCD_WIDTH - 1, 8, str);

			glcd_set_font(Verdana_11x12_Hex, 11, 12, 43, 70);
			glcd_draw_string_xy(0, 36, "DC");
			sprintf(str, "%.1f", rms);
			glcd_draw_string_xy_align_right(GLCD_LCD_WIDTH - 1, 36, str);

			glcd_tiny_set_font(Font5x7, 5, 7, 32, 127);
			glcd_tiny_draw_string_xy(0, 0, device_name);
			glcd_tiny_draw_string_xy(0, 28, "mode");
			glcd_tiny_draw_string_xy(59, 28, "peak");

			glcd_write();
		}
		// sleep_mode();
	}

	return 0;
}
