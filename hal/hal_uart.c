/**@file    hal_uart.c
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#include "hal_uart.h"
#include "board.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

/***Handle undefined constant***/
#ifndef SMCLK_F
#define SMCLK_F             1048576
#endif

#ifndef UART_BASE
//#define UART_BASE         USCI_A0_BASE
#define UART_BASE           USCI_A1_BASE
#endif

#if UART_BASE == USCI_A0_BASE
#define URAT_SET_GPIO       P3SEL |= BIT3+BIT4
#define UART_VECTOR         USCI_A0_VECTOR
#define UART_CTL1           UCA0CTL1
#define UART_BR0            UCA0BR0
#define UART_BR1            UCA0BR1
#define UART_MCTL           UCA0MCTL
#define UART_IE             UCA0IE
#define UART_IFG            UCA0IFG
#define UART_IV             UCA0IV
#define UART_TXBUF          UCA0TXBUF
#define UART_RXBUF          UCA0RXBUF
#endif

#if UART_BASE == USCI_A1_BASE
#define URAT_SET_GPIO       P4SEL |= BIT4+BIT5
#define UART_VECTOR         USCI_A1_VECTOR
#define UART_CTL1           UCA1CTL1
#define UART_BR0            UCA1BR0
#define UART_BR1            UCA1BR1
#define UART_MCTL           UCA1MCTL
#define UART_IE             UCA1IE
#define UART_IFG            UCA1IFG
#define UART_IV             UCA1IV
#define UART_TXBUF          UCA1TXBUF
#define UART_RXBUF          UCA1RXBUF
#define UART_DMA0_TX_TRIG   DMA0TSEL_21
#endif

/***UART operation constant***/
#define UART_RX_BUF_SIZE    256
#define UART_TX_BUF_SIZE    256
#define UART_PRINTF_EN      1

//Buffer
unsigned char g_Uart_RxBuffer[UART_RX_BUF_SIZE] =
{ 0 };
unsigned char g_Uart_RxCount = 0;

/***External Functions.***/
void Uart_init(unsigned long baudrate)
{
    //Calculate Baud-Rate
    unsigned long br = SMCLK_F / baudrate;
    unsigned long brs = 8 * (SMCLK_F - br * baudrate) / baudrate;

    //Initial UART
    URAT_SET_GPIO;                          // Set GPIO as TX/RX
    UART_CTL1 |= UCSWRST;                   // **Put state machine in reset**
    UART_CTL1 |= UCSSEL__SMCLK;             // Clock source = SMCLK
    UART_BR0 = br & 0xFF;                   // Baud-Rate Low byte
    UART_BR1 = br >> 8;                     // Baud-Rate High byte
    UART_MCTL |= UCBRS_1 * brs + UCBRF_0;   // Baud-Rate Second stage
    UART_CTL1 &= ~UCSWRST;                  // **Initialize USCI state machine**
    UART_IE |= UCRXIE;                      // Enable UART RX interrupt
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=UART_VECTOR
__interrupt void Uart_Isr(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(UART_VECTOR))) Uart_Isr (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(UART_IV, 4))
    {
    case 0:                         // Vector 0 - no interrupt
        break;
    case 2:                         // Vector 2 - RXIFG
        //Loop back function
        while (!(UART_IFG & UCTXIFG))
        {
            ;
        }
        UART_TXBUF = UART_RXBUF;

        //buffer received data.
        g_Uart_RxBuffer[g_Uart_RxCount++] = UART_RXBUF;
        break;
    case 4:                 // Vector 4 - TXIFG
        break;
    default:
        break;
    }
}

void Uart_putc(char byte)
{
    while (!(UART_IFG & UCTXIFG))
    {
        ;
    }
    UART_TXBUF = byte;
}

unsigned char Uart_puts(char *string)
{
    unsigned char i, len;
    len = strlen(string);
    for (i = 0; i < len; i++)
    {
        Uart_putc(string[i]);
    }
    return len;
}

unsigned char Uart_getc(void)
{
    return UART_RXBUF;
}

/*! Get all UART RX data. */
unsigned int Uart_gets(unsigned char *pdst)
{
    if (g_Uart_RxCount)
    {
        memcpy(pdst, g_Uart_RxBuffer, g_Uart_RxCount);
    }
    return g_Uart_RxCount;
}

unsigned int Uart_getl(unsigned char *pdst)
{
    unsigned int i = 0;
    unsigned int pos = 0;
    //Find '\r' position
    for (i = 0; i < g_Uart_RxCount; i++)
    {
        if (g_Uart_RxBuffer[i] == '\r')
        {
            pos = i;
        }
    }
    //Copy data before '\r'
    if(pos)
    {
        memcpy(pdst, g_Uart_RxBuffer,pos);
    }
    return pos;
}

void Uart_clear(void)
{
    memset(g_Uart_RxBuffer, 0x00, g_Uart_RxCount);
    g_Uart_RxCount = 0;
}

void Uart_print(char *fmt, ...)
{
    va_list ap;
    char string[UART_TX_BUF_SIZE];
    va_start(ap, fmt);
    vsprintf(string, fmt, ap);
    Uart_puts(string);
    va_end(ap);
}

#if UART_PRINTF_OVERRIDE
int fputc(int _c, register FILE *_fp)
{
    Uart_putc(_c);
    return (unsigned char) _c;
}

int fputs(const char *_ptr, register FILE *_fp)
{
    return Uart_puts((char*) _ptr);
}
#endif
