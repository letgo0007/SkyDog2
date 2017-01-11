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
static unsigned char I2cSlave_TxBuf[I2C_S_TXBUF_SIZE] =
{ 1, 2, 3, 4, 5, 6, 7, 8 };
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

/***[ I2C Slave ] start*******************************************************/

#ifndef I2C_MASTER_BASE
#define I2C_MASTER_BASE      USCI_B0_BASE
#endif

/*****************************************************************************
 * [ I2C Slave ]  macro & register define.
 *****************************************************************************/
#if I2C_MASTER_BASE == USCI_B0_BASE
#define SET_I2C_M_FUNC_IO   P3SEL |= BIT0+BIT1 //P3.0 = SDA , P3.1 = SCL
#define I2C_M_CTL1          UCB0CTL1
#define I2C_M_CTL0          UCB0CTL0
#define I2C_M_SA            UCB0I2CSA
#define I2C_M_BR0           UCB0BR0
#define I2C_M_BR1           UCB0BR1
#define I2C_M_IE            UCB0IE
#define I2C_M_VECTOR        USCI_B0_VECTOR
#define I2C_M_IV            UCB0IV
#define I2C_M_IFG           UCB0IFG
#define I2C_M_RXBUF         UCB0RXBUF
#define I2C_M_TXBUF         UCB0TXBUF
#define I2C_M_STAT          UCB0STAT
#endif

#if I2C_MASTER_BASE == USCI_B1_BASE
#define SET_I2C_M_FUNC_IO   P4SEL |= BIT1+BIT2  //P4.1 = SDA , P4.2 = SCL
#define I2C_M_CTL1          UCB1CTL1
#define I2C_M_CTL0          UCB1CTL0
#define I2C_M_SA            UCB1I2CSA
#define I2C_M_BR0           UCB1BR0
#define I2C_M_BR1           UCB1BR1
#define I2C_M_IE            UCB1IE
#define I2C_M_VECTOR        USCI_B1_VECTOR
#define I2C_M_IV            UCB1IV
#define I2C_M_IFG           UCB1IFG
#define I2C_M_RXBUF         UCB1RXBUF
#define I2C_M_TXBUF         UCB1TXBUF
#define I2C_M_STAT          UCB1STAT
#endif

#ifndef SMCLK_F
#define SMCLK_F         1000000
#endif
#ifndef MCLK_F
#define MCLK_F         1000000
#endif

#define I2C_M_TIMEOUT       MCLK_F / 4      //1 sec timeout.

unsigned int I2cMaster_TxCount;
unsigned int I2cMaster_RxCount;

unsigned char *I2cMaster_TxPtr;
unsigned char *I2cMaster_RxPtr;

unsigned char I2cMaster_NackFlag;

void I2cMaster_init(unsigned long clk)
{
    SET_I2C_M_FUNC_IO;                   // Assign I2C pins to USCI_B0
    I2C_M_CTL1 |= UCSWRST;                      // Enable SW reset
    I2C_M_CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    I2C_M_CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK
    I2C_M_BR0 = SMCLK_F / clk;                             // fSCL = SMCLK/12 = ~100kHz
    I2C_M_BR1 = 0;

    I2C_M_CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    I2C_M_IE |= UCRXIE + UCTXIE + UCNACKIE;                         // Enable RX interrupt
}

I2C_MASTER_RET I2cMaster_write(unsigned char slvadd, unsigned char *s, unsigned int len)
{
    //Check parameter.
    if ((s == 0) || (len == 0))
    {
        return I2C_MASTER_FAIL;
    }

    //Set 7-bit slave address
    I2C_M_SA = slvadd / 2;

    //Load transmit pointer & length.
    I2cMaster_TxPtr = s;
    I2cMaster_TxCount = len;
    I2cMaster_NackFlag = 0;

    //Send write start.
    while (I2C_M_CTL1 & UCTXSTP)
        ;             // Ensure stop condition got sent
    I2C_M_CTL1 |= UCTR + UCTXSTT;

    //Wait ISR to finish transfer with timeout
    unsigned long timeout = I2C_M_TIMEOUT;
    while (I2cMaster_TxCount && --timeout)
    {
        ;
    }

    //Return Result.
    if (timeout == 0)
    {
        return I2C_MASTER_TIMEOUT;
    }

    if (I2cMaster_NackFlag)
    {
        return I2C_MASTER_NACK;
    }

    return I2C_MASTER_SUCCESS;
}

I2C_MASTER_RET I2cMaster_read(unsigned char slvadd, unsigned char *s, unsigned int len)
{
    //Check parameter.
    if ((s == 0) || (len == 0))
    {
        return I2C_MASTER_FAIL;
    }

    //Set 7-bit slave address
    I2C_M_SA = slvadd / 2;

    //Load transmit pointer & length.
    I2cMaster_RxPtr = s;
    I2cMaster_RxCount = len;
    I2cMaster_NackFlag = 0;

    //Send read start.
    I2C_M_CTL1 &= ~UCTR;
    if (len == 1)
    {
        while (I2C_M_CTL1 & UCTXSTP)
            ;             // Ensure stop condition got sent
        I2C_M_CTL1 |= UCTXSTT;                    // I2C start condition
        while (I2C_M_CTL1 & UCTXSTT)
            ;              // Start condition sent?
        I2C_M_CTL1 |= UCTXSTP;                    // I2C stop condition
    }
    else
    {
        while (I2C_M_CTL1 & UCTXSTP)
            ;             // Ensure stop condition got sent
        I2C_M_CTL1 |= UCTXSTT;
    }

    //Wait ISR to finish transfer with timeout
    unsigned long timeout = I2C_M_TIMEOUT;
    while (I2cMaster_RxCount && --timeout)
    {
        ;
    }

    //Return Result.
    if (timeout == 0)
    {
        return I2C_MASTER_TIMEOUT;
    }

    if (I2cMaster_NackFlag)
    {
        return I2C_MASTER_NACK;
    }

    return I2C_MASTER_SUCCESS;
}

I2C_MASTER_RET I2cMaster_wread(unsigned char slvadd, unsigned char *tx, unsigned int txlen, unsigned char *rx,
        unsigned int rxlen)
{
    //Check parameter.
    if ((tx == 0) || (txlen == 0) || (rx == 0) || (rxlen == 0))
    {
        return I2C_MASTER_FAIL;
    }

    //Set 7-bit slave address
    I2C_M_SA = slvadd / 2;

    //Load transmit & receive pointer & length.
    I2cMaster_TxPtr = tx;
    I2cMaster_TxCount = txlen;
    I2cMaster_RxPtr = rx;
    I2cMaster_RxCount = rxlen;
    I2cMaster_NackFlag = 0;

    //[ Write ] : Send write start.
    while (I2C_M_CTL1 & UCTXSTP)
        ;
    I2C_M_CTL1 |= UCTR + UCTXSTT;

    //[ Write ] : Wait ISR to finish transfer with timeout
    unsigned long timeout = I2C_M_TIMEOUT;
    while (I2cMaster_TxCount && --timeout)
    {
        ;
    }

    //[ Write ] : Return Result if Error happen.
    if (timeout == 0)
    {
        return I2C_MASTER_TIMEOUT;
    }

    if (I2cMaster_NackFlag)
    {
        return I2C_MASTER_NACK;
    }

    //[ Read ] : Send read start
    __delay_cycles(10000);
    I2C_M_CTL1 &= ~UCTR;
    I2C_M_CTL1 |= UCTXSTT;                    // I2C start condition

    if (rxlen == 1)
    {
        while (I2C_M_CTL1 & UCTXSTT)
            ;              // Start condition sent?
        I2C_M_CTL1 |= UCTXSTP;                    // I2C stop condition
    }

    //[ Read ] : Wait ISR to finish receive with timeout
    timeout = I2C_M_TIMEOUT;
    while (I2cMaster_RxCount && --timeout)
    {
        ;
    }

    //[ Read ] : Return Result.
    if (timeout == 0)
    {
        return I2C_MASTER_TIMEOUT;
    }

    if (I2cMaster_NackFlag)
    {
        return I2C_MASTER_NACK;
    }

    return I2C_MASTER_SUCCESS;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = I2C_M_VECTOR
__interrupt void I2cMaster_Isr(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(I2C_M_VECTOR))) I2cMaster_Isr (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(I2C_M_IV, 12))
    {
    case 0: // Vector  0: No interrupts
        break;
    case 2: // Vector  2: ALIFG
        break;
    case 4: // Vector  4: NACKIFG
        I2cMaster_RxCount = 0;
        I2cMaster_TxCount = 0;
        I2C_M_CTL1 |= UCTXSTP;
        I2cMaster_NackFlag = 1;
        break;
    case 6: // Vector  6: STTIFG
        break;
    case 8: // Vector  8: STPIFG
        break;
    case 10: // Vector 10: RXIFG
        I2cMaster_RxCount--;
        if (I2cMaster_RxCount)
        {
            *I2cMaster_RxPtr++ = I2C_M_RXBUF;
            if (I2cMaster_RxCount == 1)
                I2C_M_CTL1 |= UCTXSTP;
        }
        else
        {
            *I2cMaster_RxPtr = I2C_M_RXBUF;
            I2C_M_IFG &= ~UCRXIFG;
        }
        break;
    case 12: // Vector 12: TXIFG
        if (I2cMaster_TxCount)
        {
            I2C_M_TXBUF = *I2cMaster_TxPtr++;
            I2cMaster_TxCount--;
        }
        else
        {
            if (I2cMaster_RxCount == 0)
            {
                I2C_M_CTL1 |= UCTXSTP;  // Stop condition only when not need restart.
            }
            I2C_M_IFG &= ~UCTXIFG;
        }
    default:
        break;
    }
}
