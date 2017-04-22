#include <stdio.h>
#include <stdio.h>
#include <string.h>

#include <common.h>

#include "utils/usart.h"
#include "utils/timebase.h"
#include "utils/debounce.h"
#include "utils/str_utils.h"

#include "init.h"
#include "blink.h"

const char *device_sw1_name = "Switch 1";
const char *device_sw2_name = "Switch 2";
int switch1 = 0;
int switch2 = 1;
int slider_red = 30;
int slider_green = 50;
int slider_blue = 70;

/** Init peripherals; Called by startup script, before main() */
void SystemInit(void)
{
	init_clock();
	init_systick();
	init_gpios();
	// init_adc();
	// init_dac();
	init_pwm1();
	usart_init(USART2, 460800);
}

/* usart2 interrupt handler */
void USART2_IRQHandler(void)
{
	usart_handle_interrupts(USART2);
}

void handle_line(char *line)
{
	int len = strlen(line);
	if (strcmp(line, "get:config") == 0)
	{
		usart_qprintf(USART2, "config:channel:B:name:%s\n", device_sw1_name);
		usart_qprintf(USART2, "config:channel:B:type:switch\n");
		usart_qprintf(USART2, "config:channel:B:mode:sink\n");
		usart_qprintf(USART2, "config:channel:B:method:push\n");
		usart_qprintf(USART2, "config:channel:C:name:%s\n", device_sw2_name);
		usart_qprintf(USART2, "config:channel:C:type:switch\n");
		usart_qprintf(USART2, "config:channel:C:mode:sink\n");
		usart_qprintf(USART2, "config:channel:C:method:push\n");
		usart_qprintf(USART2, "config:channel:D:static:name:Slider Red\n");
		usart_qprintf(USART2, "config:channel:D:type:slider\n");
		usart_qprintf(USART2, "config:channel:D:mode:sink\n");
		usart_qprintf(USART2, "config:channel:D:method:push\n");
		usart_qprintf(USART2, "config:channel:D:color:000000:ff0000\n");
		usart_qprintf(USART2, "config:channel:D:parent:C\n");
		usart_qprintf(USART2, "config:channel:E:static:name:Slider Green\n");
		usart_qprintf(USART2, "config:channel:E:type:slider\n");
		usart_qprintf(USART2, "config:channel:E:mode:sink\n");
		usart_qprintf(USART2, "config:channel:E:method:push\n");
		usart_qprintf(USART2, "config:channel:E:color:000000:00ff00\n");
		usart_qprintf(USART2, "config:channel:E:parent:C\n");
		usart_qprintf(USART2, "config:channel:F:static:name:Slider Blue\n");
		usart_qprintf(USART2, "config:channel:F:type:slider\n");
		usart_qprintf(USART2, "config:channel:F:mode:sink\n");
		usart_qprintf(USART2, "config:channel:F:method:push\n");
		usart_qprintf(USART2, "config:channel:F:color:000000:0000ff\n");
		usart_qprintf(USART2, "config:channel:F:parent:C\n");
		usart_qprintf(USART2, "B%d\n", switch1);
		usart_qprintf(USART2, "C%d\n", switch2);
		usart_qprintf(USART2, "D%x\n", slider_red);
		usart_qprintf(USART2, "E%x\n", slider_green);
		usart_qprintf(USART2, "F%x\n", slider_blue);
	}
	else if (line[0] == 'C' && len > 1)
	{
		switch2 = line[1] == '0' ? 0 : 1;
		TIM4_CCR1 = switch2 ? slider_red : 0;
		TIM4_CCR2 = switch2 ? slider_green : 0;
		TIM4_CCR3 = switch2 ? slider_blue : 0;
	}
	else if (line[0] == 'D' && len > 1)
	{
		slider_red = (int)strtol(line + 1, NULL, 16);
		TIM4_CCR1 = switch2 ? slider_red : 0;
	}
	else if (line[0] == 'E' && len > 1)
	{
		slider_green = (int)strtol(line + 1, NULL, 16);
		TIM4_CCR2 = switch2 ? slider_green : 0;
	}
	else if (line[0] == 'F' && len > 1)
	{
		slider_blue = (int)strtol(line + 1, NULL, 16);
		TIM4_CCR3 = switch2 ? slider_blue : 0;
	}
}

int main(void)
{
	TIM4_CCR1 = slider_red;
	TIM4_CCR2 = slider_green;
	TIM4_CCR3 = slider_blue;
	usart_qprintf(USART2, "B%d\n", switch1);
	usart_qprintf(USART2, "C%d\n", switch2);
	usart_qprintf(USART2, "D%x\n", slider_red);
	usart_qprintf(USART2, "E%x\n", slider_green);
	usart_qprintf(USART2, "F%x\n", slider_blue);

	while (1)
	{
		char *line = usart_qget();
		if (line)
		{
			green_toggle();
			handle_line(line);
			free(line);
		}
		// delay_ms(500);
		// green_toggle();
		// usart_qprintf(USART2, "hi\r\n");
	}
}
