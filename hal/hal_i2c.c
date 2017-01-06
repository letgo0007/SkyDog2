/**@file    hal_i2c.c
 *
 * MSP430F5XXX I2C (USCI_Bx) HAL.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170105 | Yang.Zf | Initial Version
 */

#include "hal_i2c.h"
#include "board.h"

/***[ I2C Slave ] start*******************************************************/

#ifndef I2C_SLAVE_BASE
#define I2C_SLAVE_BASE      USCI_B0_BASE
#endif

/*****************************************************************************
 * [ I2C Slave ]  macro & register define.
 *****************************************************************************/
#if I2C_SLAVE_BASE == USCI_B0_BASE
#define SET_I2C_S_FUNC_IO   P3SEL |= BIT0+BIT1
#define I2C_S_CTL1          UCB0CTL1
#define I2C_S_CTL0          UCB0CTL0
#define I2C_S_OA            UCB0I2COA
#define I2C_S_IE            UCB0IE
#define I2C_S_VECTOR        USCI_B0_VECTOR
#define I2C_S_IV            UCB0IV
#define I2C_S_IFG           UCB0IFG
#define I2C_S_RXBUF         UCB0RXBUF
#define I2C_S_TXBUF         UCB0TXBUF
#define I2C_S_STAT          UCB0STAT
#endif

#if I2C_SLAVE_BASE == USCI_B1_BASE

#endif
#define I2C_S_RXBUF_SIZE    128
#define I2C_S_TXBUF_SIZE    128

/*****************************************************************************
 * [ I2C Slave ]  operation buffers.
 *****************************************************************************/

static unsigned int I2cSlave_RxCount;
static unsigned int I2cSlave_TxCount;

unsigned char *I2cSlave_RxPtr = 0;
unsigned char *I2cSlave_TxPtr = 0;

static unsigned char I2cSlave_RxBuf[I2C_S_RXBUF_SIZE];
static unsigned char I2cSlave_TxBuf[I2C_S_TXBUF_SIZE];
/*****************************************************************************
 * [ I2C Slave ]  Internal functions.
 *****************************************************************************/

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = I2C_S_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(I2C_S_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(I2C_S_IV, 12))
    {
    case 0:     // Vector  0: No interrupts
        break;
    case 2:     // Vector  2: ALIFG
        break;
    case 4:     // Vector  4: NACKIFG
        break;
    case 6:     // Vector  6: STTIFG
        I2C_S_IFG &= ~UCSTTIFG;
        I2cSlave_RxCount = 0;
        I2cSlave_TxCount = 0;
        break;
    case 8:     // Vector  8: STPIFG
        break;
    case 10:     // Vector 10: RXIFG
        I2cSlave_RxBuf[I2cSlave_RxCount] = I2C_S_RXBUF;
        I2cSlave_RxCount++;
        break;
    case 12:     // Vector 12: TXIFG
        I2C_S_TXBUF = I2cSlave_TxBuf[I2cSlave_TxCount];
        I2cSlave_TxCount++;
        break;
    default:
        break;
    }
}

/*****************************************************************************
 * [ I2C Slave ]  External functions.
 *****************************************************************************/
void I2cSlave_init(unsigned char slave_add)
{
    SET_I2C_S_FUNC_IO;
    I2C_S_CTL1 |= UCSWRST;                  // Reset USCI
    I2C_S_CTL0 = UCMODE_3 + UCSYNC;         // I2C Slave, synchronous mode
    I2C_S_OA = slave_add;                   // Slave Address
    I2C_S_CTL1 &= ~UCSWRST;                 // Enable USCI
    I2C_S_IE |= UCSTPIE + UCSTTIE + UCRXIE + UCTXIE;     // Enable interrupt.
}

unsigned char I2cSlave_getc(void)
{
    return I2C_S_RXBUF;
}

unsigned int I2cSlave_gets(unsigned char *s)
{
    if (I2C_S_STAT & UCBBUSY)   //I2C is busy.
    {
        return 0;
    }

    unsigned int i;
    for (i = 0; i < I2cSlave_RxCount; i++)
    {
        *s++ = I2cSlave_RxBuf[i];
    }
    return I2cSlave_RxCount;
}

void I2cSlave_putc(unsigned char c)
{
    I2C_S_TXBUF = c;
}

void I2cSlave_puts(unsigned char *s, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < I2cSlave_RxCount; i++)
    {
        I2cSlave_TxBuf[i] = *s++;
    }
}

