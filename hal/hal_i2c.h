/**@file    hal_i2c.h
 *
 * MSP430F5XXX I2C (USCI_Bx) HAL.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170105 | Yang.Zf | Initial Version
 */

#ifndef HAL_HAL_I2C_H_
#define HAL_HAL_I2C_H_

#include <msp430.h>

/*!@brief   Initialize I2C Slave.
 * @param   slave_add   :[0~0x7F] Slave address in 7bit.
 */
extern void I2cSlave_init(unsigned char slave_add);

/*!@brief   Return 1 byte (the last received) from I2C .
 */
extern unsigned char I2cSlave_getc(void);

/*!@brief   Put 1 char to I2C Slave , wait master to read.
 * @param   c   : 1 byte of data.
 */
extern void I2cSlave_putc(unsigned char c);

/*!@brief   Copy I2C received data out.
 *
 * @param   s   : is the pointer of buffer to copy I2C data.
 * @return  len : the length of bytes received.
 *          0x00: No Data received or I2C is busy.
 */
extern unsigned int I2cSlave_gets(unsigned char *s);

/*!@brief   Put multiple bytes to I2C Slave , wait master to read.
 * @param   s   : is the pointer to data.
 * @param   len : is the length of data.
 */
extern void I2cSlave_puts(unsigned char *s, unsigned int len);

typedef enum I2C_MASTER_RET
{
    I2C_MASTER_SUCCESS = 0x00, I2C_MASTER_FAIL = 0x01 ,I2C_MASTER_TIMEOUT = 0x02, I2C_MASTER_NACK = 0x03
} I2C_MASTER_RET;

void I2cMaster_init(unsigned long clk);

I2C_MASTER_RET I2cMaster_write(unsigned char slvadd, unsigned char *s, unsigned int len);

I2C_MASTER_RET I2cMaster_read(unsigned char slvadd, unsigned char *s, unsigned int len);

I2C_MASTER_RET I2cMaster_wread(unsigned char slvadd, unsigned char *tx, unsigned int txlen, unsigned char *rx,
        unsigned int rxlen);

#endif /* HAL_HAL_I2C_H_ */
