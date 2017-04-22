
#include <math.h>

#include "init.h"

#include "utils/gpio.h"
#include "utils/usart.h"
#include "utils/nvic.h"

// This file contains examples of initializing some peripherals


void init_gpios(void)
{
	gpio_enable(GPIOC);

	// LEDs
	gpio_set_mode(GPIOC, BIT8 | BIT9, MODER_OUTPUT);
}

/*
Increasing the CPU frequency (in the same voltage range).
• Program the 64-bit access by setting the ACC64 bit in FLASH_ACR
• Check that 64-bit access is taken into account by reading FLASH_ACR
• Program 1 WS to the LATENCY bit in FLASH_ACR
• Check that the new number of WS is taken into account by reading FLASH_ACR
• Modify the CPU clock source by writing to the SW bits in the RCC_CFGR register
• If needed, modify the CPU clock prescaler by writing to the HPRE bits in RCC_CFGR
• Check that the new CPU clock source or/and the new CPU clock prescaler value is/are
taken into account by reading the clock source status (SWS bits) or/and the AHB
prescaler value (HPRE bits), respectively, in the RCC_CFGR register
*/
void init_clock(void)
{
	/* flash timing - 64-bit access, pre-fetch, latency = 1 */
	FLASH_ACR |= FLASH_ACR_ACC64; /* this bit cannot be written at the same time as the LATENCY and PRFTEN bits */
	while (!(FLASH_ACR & FLASH_ACR_ACC64));
	FLASH_ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY;
	while (!(FLASH_ACR & FLASH_ACR_PRFTEN) || !(FLASH_ACR & FLASH_ACR_LATENCY));

	/* setup pll */
	RCC_CFGR |= RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL4 | RCC_CFGR_PLLDIV2;

	/* power on clock(s) */
	RCC_CR |= RCC_CR_HSION | RCC_CR_PLLON;

	/* wait for clock(s) to start */
	while (!(RCC_CR & RCC_CR_HSIRDY) || !(RCC_CR & RCC_CR_PLLRDY));

	/* select HSI as the core clock source */
	RCC_CFGR &= ~RCC_CFGR_SW;
	RCC_CFGR |= RCC_CFGR_SW_PLL;
}


void init_systick(void)
{
	patch_register(SysTick_CSR, SysTick_CSR_CLKSOURCE, 1); // 1 - core, 0 - div 8

	SysTick_RELOAD = 16000; // 1ms interrupt @ 16MHz core clock
	SysTick_CSR |= SysTick_CSR_TICKINT | SysTick_CSR_ENABLE;
}

// ----


void init_adc(void)
{
	gpio_set_mode(GPIOB, BIT1 | BIT0 | BIT12, MODER_ANALOG); // configure pins you want to use

	// enable clock for ADC
	RCC_APB2ENR |= RCC_APB2ENR_ADC1EN;

	// 12-bit, right aligned
	ADC1_CR1 = 0;
	ADC1_CR2 = 0;

	// enable internal temperature sensor (16) & voltage reference (17)
	ADC_CCR |= ADC_CCR_TSVREFE;

	ADC1_SMPR3 = 0b100; // sample time

	// turn ADC on
	ADC1_CR2 |= ADC_CR2_ADON;
	// Wait for ADONS
	while (!(ADC1_SR & ADC_SR_ADONS));
}

// EXAMPLE to measure an ADC channel
uint16_t adc_measure(uint8_t channel)
{
	ADC1_SQR5 = channel; // select n-th channel
	ADC1_SQR1 = 0; // sets length to 1 (0000)

	ADC1_CR2 |= ADC_CR2_SWSTART; // start conversion

	// wait for end of conversion
	while (!(ADC1_SR & ADC_SR_EOC));
	return ADC1_DR;
}


// ----


void init_dac(void)
{
	RCC_APB1ENR |= RCC_APB1ENR_DACEN; // enable DAC

	gpio_set_mode(GPIOA, BIT4, MODER_ANALOG); // PA4 - DAC CH1 out

	DAC_CR |= DAC_CR_EN1; // enable first channel

	DAC_DHR12R1 = 0; // reset value
}


void init_pwm1(void)
{
	// enable clock for the timer
	RCC_APB1ENR |= RCC_APB1ENR_TIM4EN;

	gpio_set_af(GPIOB, BIT6, AF2);
	gpio_set_af(GPIOB, BIT7, AF2);
	gpio_set_af(GPIOB, BIT8, AF2);

	patch_register(TIM4_CCMR1, TIM_CCMR1_OC1M, TIM_OCM_PWM1); // set PWM1 mode
	patch_register(TIM4_CCMR1, TIM_CCMR1_OC2M, TIM_OCM_PWM1); // set PWM1 mode
	patch_register(TIM4_CCMR2, TIM_CCMR2_OC3M, TIM_OCM_PWM1); // set PWM1 mode

	TIM4_CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE; // preload enable
	TIM4_CCMR2 |= TIM_CCMR2_OC3PE; // preload enable
	TIM4_CR1 |= TIM_CR1_ARPE; // auto reload is buffered
	TIM4_CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E; // enable output compare (PWM output)

	patch_register(TIM4_CR1, TIM_CR1_CMS, TIM_CMS_EDGE); // centering mode
	patch_register(TIM4_CR1, TIM_CR1_DIR, 0); // count upwards only

	// frequency set to 16 kHz

	TIM4_PSC = 0; // prescaller
	TIM4_ARR = 1000; // sets frequency
	TIM4_CCR1 = 0; // duty cycle
	TIM4_CCR2 = 0; // duty cycle
	TIM4_CCR3 = 0; // duty cycle

	// generate update event to latch the value registers
	TIM4_EGR |= TIM_EGR_UG;
	TIM4_CR1 |= TIM_CR1_CEN; // enable timer.
}
