/**@file    hal_uart.h
 *
 * MSP430F5XXX UART (USCI module) HAL.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#ifndef HAL_HAL_UART_H_
#define HAL_HAL_UART_H_

#include <msp430.h>
#include <stdio.h>

#define UART_PRINTF_OVERRIDE    1       //1: printf override to UART  0: printf route to JTAG
#define UART_RX_BUF_SIZE        256     //TX buffer size. Maximum size for sending at 1 time.
#define UART_TX_BUF_SIZE        256     //TX buffer size. Maximum size for receiving at 1 time.

/******************************************************************************
 * @fn      Uart_init
 * @brief   Initialize MSP430 UART ( USCI_A0 or USCI_A1 ).
 * @param   baudrate    is the baudrate of UART.
 *****************************************************************************/
extern void Uart_init(unsigned long baudrate);

/******************************************************************************
 * @fn      Uart_putc
 * @brief   Put 1 char to UART TX.
 * @param   byte    the byte to be transmitted.
 *****************************************************************************/
extern void Uart_putc(char byte);

/******************************************************************************
 * @fn      Uart_puts
 * @brief   Put string to UART TX.
 * @param   string    the pointer string to be transmitted.
 *****************************************************************************/
extern unsigned char Uart_puts(char *string);

/******************************************************************************
 * @fn      Uart_getc
 * @brief   Get 1 char from UART RX.
 * @return  The last char received at UART RX.
 *****************************************************************************/
extern unsigned char Uart_getc(void);

/******************************************************************************
 * @fn      Uart_gets
 * @brief   Get string from UART RX (buffered).
 * @note    If buffer is full ,data will be override.
 * @param   s is the pointer to destination of data copy buffer.
 * @return  The number of bytes received.
 *****************************************************************************/
extern unsigned int Uart_gets(unsigned char *s);

/******************************************************************************
 * @fn      Uart_getl
 * @brief   Get 1 line of string ,till get '\r'.
 * @param   s is the pointer to destination of data copy buffer.
 * @return  The number of bytes received.
 *****************************************************************************/
extern unsigned int Uart_getl(unsigned char *s);

/******************************************************************************
 * @fn      Uart_clear
 * @brief   Clear UART buffer & count.
 *****************************************************************************/
extern void Uart_clear(void);

/******************************************************************************
 * @fn      Uart_print
 * @brief   C/C++ style print function , usage is the same as printf().
 * @note    Only %x %d %s are supported.
 * @example Uart_print("This is a test %d .\r" , i++);
 *****************************************************************************/
void Uart_print(char *fmt, ...);

#if UART_PRINTF_OVERRIDE
/******************************************************************************
 * @fn      fputc()
 * @fn      fputs()
 * @brief   C/C++ style print function , usage is the same as printf().
 *          \def UART_PRINTF_OVERRIDE == 0 , by default on MSP430 device,
 *          printf() will be routed to JTAG interface.
 *          \def UART_PRINTF_OVERRIDE == 1 , by redefine fputc() & fputs(),
 *          printf() will be routed to UART interface.
 *****************************************************************************/
extern int fputc(int _c, register FILE *_fp);
extern int fputs(const char *_ptr, register FILE *_fp);
#endif

#endif /* HAL_HAL_UART_H_ */
