/**@file    hal_spi.h
 *
 * MSP430 SPI Master/Slave HAL.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#ifndef HAL_HAL_SPI_H_
#define HAL_HAL_SPI_H_

#include "msp430.h"

#define SPI_S_RX_BUF_SIZE       256

/******************************************************************************
 * @fn      SpiSlave_init
 * @brief   Initialize SPI Slave , include USCI & DMA .
 *****************************************************************************/
void SpiSlave_init(void);

/******************************************************************************
 * @fn      SpiSlave_putc
 * @brief   Put 1 byte to SPI Slave MISO.
 * @param   c   is the byte to be transmitted.
 *****************************************************************************/
void SpiSlave_putc(char c);

/******************************************************************************
 * @fn      SpiSlave_puts
 * @brief   Put string to SPI Slave MISO.
 * @note    The pointer of data is set , the actual data transfer is handled by
 *          SPI Slave TX ISR function when SPI master clock reaches.
 * @param   *s   is the pointer to string to be transmitted.
 * @param   len  is the length of string.
 *****************************************************************************/
void SpiSlave_puts(unsigned char *s, unsigned int len);

/******************************************************************************
 * @fn      SpiSlave_getc
 * @brief   Get 1 byte (the last received) from SPI Slave MOSI.
 * @return  Last byte received at MOSI.
 *****************************************************************************/
unsigned char SpiSlave_getc(void);

/******************************************************************************
 * @fn      SpiSlave_gets
 * @brief   SPI slave MOSI received data is buffered to RAM by DMA.
 *          This function copy buffered data out & return the number of byte received.
 * @param   *s   : is the pointer to destination of copy.
 * @return  \b 0 : No byte received or SPI_SLAVE_CS is Low (MOSI is transmitting).
 *          \b 1~255 : Number of byte received.
 *****************************************************************************/
unsigned int SpiSlave_gets(unsigned char *s);

/******************************************************************************
 * @fn      SpiSlave_clear
 * @brief   Clear SPi Slave Received buffer, reset DMA .
 *****************************************************************************/
void SpiSlave_clear(void);

/******************************************************************************
 * @fn      SpiMaster_init
 * @brief   Initialize SPI Master.
 * @note    SPI_M_CS_ALL pin is set to GPIO output by this function.
 *          Other SPI_M_CS pins need to be initialized as GPIO output elsewhere.
 *****************************************************************************/
void SpiMaster_init(unsigned long freq);

/******************************************************************************
 * @fn      SpiMaster_putc
 * @brief   Put 1 byte to SPI Master MOSI.
 * @param   c   : is the byte to be transmitted.
 *****************************************************************************/
void SpiMaster_putc(char c);

/******************************************************************************
 * @fn      SpiMaster_puts
 * @brief   Put string to SPI Master MOSI.
 * @param   *s   : is the pointer to string to be transmitted.
 * @param   len  : is the size of string.
 *****************************************************************************/
void SpiMaster_puts(unsigned char *s, unsigned int len);

/******************************************************************************
 * @fn      SpiMaster_getc
 * @brief   Get 1 byte (Last byte received) from SPI Master MISO.
 * @return  is the last byte to be received.
 *****************************************************************************/
unsigned char SpiMaster_getc(void);

/******************************************************************************
 * @fn      SpiMaster_gets
 * @brief   Get multiple bytes form SPI Master MISO.
 * @note    In this function , only the pointer of received buffer is set ,
 *          the actual data transfer is handled by Spi_Master_Isr function
 *          when SPI master clock is send.
 * @example //Get 10 byte form MISO while sending data through MOSI.
 *          SpiMaster_gets(buffer,10);
 *          SpiMaster_puts("1234567890",10);
 *****************************************************************************/
void SpiMaster_gets(unsigned char *s, unsigned int len);

/******************************************************************************
 * @fn      SpiMaster_setCsPins
 * @brief   Set select CS pins to active state.
 * @param   cs_sel : Each bit of cs_sel indicates 1 SPI_M_CS pin.
 *                   SPI_M_CS_ALL is set when any bit is 1.
 *****************************************************************************/
void SpiMaster_setCsPins(unsigned int cs_sel);

#endif /* HAL_HAL_SPI_H_ */
