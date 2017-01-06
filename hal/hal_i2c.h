/**@file    hal_i2c.h
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 2017Äê1ÔÂ5ÈÕ | GKX100052 | Initial Version
 */

#ifndef HAL_HAL_I2C_H_
#define HAL_HAL_I2C_H_

#include <msp430.h>
#include "board.h"

void I2cSlave_init(unsigned char slave_add);

unsigned char I2cSlave_getc(void);

unsigned int I2cSlave_gets(unsigned char *s);

void I2cSlave_putc(unsigned char c);

void I2cSlave_puts(unsigned char *s , unsigned int len);



#endif /* HAL_HAL_I2C_H_ */
