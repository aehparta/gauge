/*
 * I2C drivers.
 */

#ifndef __I2C_H__
#define __I2C_H__

#include <avr/io.h>
#include <avr/delay.h>
#include <stdint.h>

/** \addtogroup I2C
 * @{
 */

#ifndef F_I2C
/**
 * I2C clock in hertz.
 */
#define F_I2C               400000
#endif

/* I2C clock half wave in CPU clocks */
#define I2C_DELAY()         _delay_loop_1(F_CPU / F_I2C / 3 / 2)

/* I2C SCL port and pin definitions */
#define I2C_SCL_PORT        PORTC
#define I2C_SCL_DDR         DDRC
#define I2C_SCL_IO          5

/* I2C SDA port and pin definitions */
#define I2C_SDA_PORT        PORTC
#define I2C_SDA_DDR         DDRC
#define I2C_SDA_PIN         PINC
#define I2C_SDA_IO          4

/* SCL macros */
#define I2C_SCL_OUTPUT()    (I2C_SCL_DDR |= _BV(I2C_SCL_IO))
#define I2C_SCL_LOW()       (I2C_SCL_PORT &=~_BV(I2C_SCL_IO))
#define I2C_SCL_HIGH()      (I2C_SCL_PORT |= _BV(I2C_SCL_IO))
/* SDA macros */
#define I2C_SDA_INPUT()     (I2C_SDA_DDR &=~_BV(I2C_SDA_IO))
#define I2C_SDA_OUTPUT()    (I2C_SDA_DDR |= _BV(I2C_SDA_IO))
#define I2C_SDA_LOW()       (I2C_SDA_PORT &=~_BV(I2C_SDA_IO))
#define I2C_SDA_HIGH()      (I2C_SDA_PORT |= _BV(I2C_SDA_IO))
#define I2C_SDA_READ()      (I2C_SDA_PIN & _BV(I2C_SDA_IO))

/* I2C write zero */
#define I2C_WRITE_ZERO() \
do { \
    I2C_SDA_LOW(); \
    I2C_SCL_HIGH(); \
    I2C_DELAY(); \
    I2C_SCL_LOW(); \
    I2C_DELAY(); \
} while (0);

/* I2C write one */
#define I2C_WRITE_ONE() \
do { \
    I2C_SDA_HIGH(); \
    I2C_SCL_HIGH(); \
    I2C_DELAY(); \
    I2C_SCL_LOW(); \
    I2C_DELAY(); \
} while (0);

/* I2C write bit */
#define I2C_WRITE_BIT(bit) \
do { \
    if (bit) I2C_SDA_HIGH(); \
    else I2C_SDA_LOW(); \
    I2C_SCL_HIGH(); \
    I2C_DELAY(); \
    I2C_SCL_LOW(); \
    I2C_DELAY(); \
} while (0);

/* I2C write bit from byte */
#define I2C_WRITE(byte, bit) \
do { \
    if (byte & _BV(bit)) I2C_SDA_HIGH(); \
    else I2C_SDA_LOW(); \
    I2C_SCL_HIGH(); \
    I2C_DELAY(); \
    I2C_SCL_LOW(); \
    I2C_DELAY(); \
} while (0);

/* I2C read bit */
static inline uint8_t I2C_READ_BIT(void)
{
	uint8_t bit;
	I2C_SCL_HIGH();
	I2C_DELAY();
	bit = I2C_SDA_READ();
	I2C_SCL_LOW();
	I2C_DELAY();
	return bit;
}

/* I2C read bit to byte */
#define I2C_READ(byte, bit) \
do { \
    I2C_SCL_HIGH(); \
    I2C_DELAY(); \
    byte |= ((I2C_SDA_PIN >> I2C_SDA_IO) & 0x1) << bit; \
    I2C_SCL_LOW(); \
    I2C_DELAY(); \
} while (0);

/**
 * Initialize I2C bitbang.
 */
void i2c_init(void);

int i2c_addr7(uint8_t addr, uint8_t rw);

uint8_t i2c_read_byte(int nack);

int i2c_write_byte(uint8_t byte);

void i2c_stop(void);


/** @} end group I2C */

#endif /* __I2C_H__ */
