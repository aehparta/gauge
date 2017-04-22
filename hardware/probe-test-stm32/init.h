#pragma once
#include <common.h>

void init_clock(void);
void init_gpios(void);
void init_usart(uint32_t baudrate);
void init_systick(void);
void init_adc(void);
void init_dac(void);
void init_pwm1(void);
