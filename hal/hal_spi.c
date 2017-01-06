/**@file    hal_spi.c
 *
 * MSP430 SPI Master/Slave HAL.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#include "hal_spi.h"

/***[ SPI Slave ] start*******************************************************/

#ifndef SPI_SLAVE_BASE
#define SPI_SLAVE_BASE      USCI_A0_BASE
#endif

/*****************************************************************************
 * [ SPI Slave ]  macro & register define.
 *****************************************************************************/
#if SPI_SLAVE_BASE == USCI_A0_BASE
//P3.2 = STE , P3.3 = MOSI , P3.4 = MISO , 2.7 = CLK
#define SET_SPI_S_FUNC_IO       do{ \
    P3DIR &= ~BIT2; \
    P3SEL |= BIT2 + BIT3 + BIT4; \
    P2DIR |= BIT7; \
    P2SEL |= BIT7; \
}while(0)
#define GET_SPI_S_CS_LEVEL      (P3IN & BIT2)
#define GET_SPI_S_CLK_LEVEL     (P2IN & BIT7)
#define SPI_S_CTL0              UCA0CTL0
#define SPI_S_CTL1              UCA0CTL1
#define SPI_S_TXBUF             UCA0TXBUF
#define SPI_S_RXBUF             UCA0RXBUF
#define SPI_S_IFG               UCA0IFG
#define SPI_S_IE                UCA0IE
#define SPI_S_VECTOR            USCI_A0_VECTOR
#define SPI_S_IV                UCA0IV
#define SPI_S_DMA_RX_TRIG       DMA0TSEL_16          //UCA0 RX
#define SPI_S_DMA_TX_TRIG       DMA0TSEL_17          //UCA0 TX
#endif
#if SPI_SLAVE_BASE == USCI_A1_BASE
#endif
#if SPI_SLAVE_BASE == USCI_B0_BASE
#endif
#if SPI_SLAVE_BASE == USCI_B1_BASE
#endif

#define SPI_S_RX_DMACTL0        DMACTL0
#define SPI_S_RX_DMASZ          DMA0SZ
#define SPI_S_RX_DMACTL         DMA0CTL
#define SPI_S_RX_DMASA          DMA0SA
#define SPI_S_RX_DMADA          DMA0DA

/*****************************************************************************
 * [ SPI Slave ] operation buffers.
 *****************************************************************************/
#define SPI_S_RX_BUF_SIZE       256
static unsigned char SpiS_Rxbuf[SPI_S_RX_BUF_SIZE] =
{ 0 };
static unsigned char *SpiS_TxPtr = 0;
static unsigned int SpiS_TxLen = 0;

/*****************************************************************************
 * [ SPI Slave ] Internal Functions.
 *****************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=SPI_S_VECTOR
__interrupt void SpiSlave_Isr(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(SPI_S_VECTOR))) SpiSlave_Isr (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(SPI_S_IV, 4))
    {
    case 0:     // Vector 0 - no interrupt
        break;
    case 2:     // Vector 2 - RXIFG
        break;
    case 4:     // Vector 4 - TXIFG
        if (SpiS_TxLen == 0)
        {
            SPI_S_IE &= ~UCTXIE;    //Disable TX Interrupt when finish.
        }
        else
        {
            SPI_S_TXBUF = *SpiS_TxPtr++;
            SpiS_TxLen--;
        }
        break;
    default:
        break;
    }
}

/*****************************************************************************
 * [ SPI Slave ] External Functions.
 *****************************************************************************/
void SpiSlave_init(void)
{
    /***[2] Initialize USCI as SPI Slave ******************************/
    // Set function IO
    SET_SPI_S_FUNC_IO
    ;
    // **Disable**
    SPI_S_CTL1 |= UCSWRST;
    /* 1st edge capture
     * Rising edge capture
     * MSB first
     * 4-wire mode , SPI_CS active low
     * SPI mode*/
    SPI_S_CTL0 |= 0 * UCCKPH + UCCKPL + UCMSB + UCMODE_2 + UCSYNC;
    // **Enable**
    SPI_S_CTL1 &= ~UCSWRST;

    /***[2] Initialize DMA *******************************************/
    //Set trigger source = SPI RX
    SPI_S_RX_DMACTL0 &= 0xFF00;
    SPI_S_RX_DMACTL0 |= SPI_S_DMA_RX_TRIG;
    //Set max transfer size.
    SPI_S_RX_DMASZ = SPI_S_RX_BUF_SIZE;
    //Source address = USCIxRXBUF
    __data16_write_addr((unsigned short) &SPI_S_RX_DMASA, (unsigned long) &SPI_S_RXBUF);
    //Destination address = RAM Buffer
    __data16_write_addr((unsigned short) &SPI_S_RX_DMADA, (unsigned long) &SpiS_Rxbuf);
    /* Repeated Single transfer
     * Source Address  unchanged
     * Destination Address  increase
     * Byte transfer
     * Level Trigger
     * Enable Transfer*/
    SPI_S_RX_DMACTL = DMADT_4 + DMASRCINCR_0 + DMADSTINCR_3 + DMASBDB + DMALEVEL + DMAEN;
}

void SpiSlave_putc(char c)
{
    //This function is disabled when SpiSlave_puts() is in progress.
    if (!(SPI_S_IE & UCTXIE))
    {
        SPI_S_TXBUF = c;
    }
}

void SpiSlave_puts(unsigned char *s, unsigned int len)
{
    if ((len) &&(s != 0))
    {
        //Set 1st byte.
        SPI_S_TXBUF = *s;
        //2nd ~ N byte are handled by SpiSlave_Isr();
        SpiS_TxPtr = s + 1;
        SpiS_TxLen = len - 1;
        //Enable TX interrupt
        SPI_S_IFG &= ~UCTXIFG;
        SPI_S_IE |= UCTXIE;
    }
}

unsigned char SpiSlave_getc(void)
{
    return SPI_S_RXBUF;
}

unsigned int SpiSlave_gets(unsigned char *s)
{
    if (!GET_SPI_S_CS_LEVEL)
    {
        return 0;     //NOT Allow data copy when SPI_CS = L
    }

    unsigned int i, len;
    len = SPI_S_RX_BUF_SIZE - DMA0SZ;
    for (i = 0; i < len; i++)
    {
        s[i] = SpiS_Rxbuf[i];
    }
    return len;
}

void SpiSlave_clear(void)
{
    //memset(SpiS_Rxbuf, 0x0000, sizeof(SpiS_Rxbuf));
    SPI_S_RX_DMACTL &= ~DMAEN;
    SPI_S_RX_DMACTL |= DMAEN;
}

/***[ SPI Master ] start******************************************************/

#ifndef SPI_MASTER_BASE
#define SPI_MASTER_BASE      USCI_B1_BASE
#endif

/*****************************************************************************
 * [ SPI Master ]  macro & register define.
 * \note    SPI master work in 3-wire mode. SPI_CS pins are handled as GPIO.
 *****************************************************************************/
#if SPI_MASTER_BASE == USCI_B0_BASE
//P3.0 = MOSI , P3.1 = MISO , P3.2 = CLK .
#define SET_SPI_M_FUNC_IO       P3SEL | = BIT0 + BIT1 + BIT2
#define SPI_M_CTL0              UCB0CTL0
#define SPI_M_CTL1              UCB0CTL1
#define SPI_M_BR0               UCB0BR0
#define SPI_M_BR1               UCB0BR1
#define SPI_M_STAT              UCB0STAT
#define SPI_M_TXBUF             UCB0TXBUF
#define SPI_M_RXBUF             UCB0RXBUF
#define SPI_M_IFG               UCB0IFG
#define SPI_M_IE                UCB0IE
#define SPI_M_IV                UCB0IV
#define SPI_M_VECTOR            USCI_B0_VECTOR
#endif
#if SPI_MASTER_BASE == USCI_B1_BASE
//P4.1 = MOSI , P4.2 = MISO , P4.3 = CLK , P4.0 = CS . CS is set as GPIO out.
#define SET_SPI_M_FUNC_IO       do{\
P4SEL |= BIT1 + BIT2 + BIT3; \
P4DIR |= BIT0; \
P4SEL &= ~BIT0; \
}while(0)

#define SET_SPI_M_CS_ALL_HIGH   P4OUT |= BIT0
#define SET_SPI_M_CS_ALL_LOW    P4OUT &= ~BIT0
#define SPI_M_CTL0              UCB1CTL0
#define SPI_M_CTL1              UCB1CTL1
#define SPI_M_BR0               UCB1BR0
#define SPI_M_BR1               UCB1BR1
#define SPI_M_STAT              UCB1STAT
#define SPI_M_TXBUF             UCB1TXBUF
#define SPI_M_RXBUF             UCB1RXBUF
#define SPI_M_IFG               UCB1IFG
#define SPI_M_IE                UCB1IE
#define SPI_M_IV                UCB1IV
#define SPI_M_VECTOR            USCI_B1_VECTOR
#endif

/*****************************************************************************
 * [ SPI Master ] operation buffers.
 *****************************************************************************/
static unsigned char *Spi_M_RxPtr = 0;
static unsigned char Spi_M_RxLen = 0;

/*****************************************************************************
 * [ SPI Master ] Internal Functions.
 *****************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=SPI_M_VECTOR
__interrupt void SpiMaster_Isr(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(SPI_M_VECTOR))) SpiMaster_Isr (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(SPI_M_IV, 4))
    {
    case 0:     // Vector 0 - no interrupt
        break;
    case 2:     // Vector 2 - RXIFG
        if (Spi_M_RxLen == 0)
        {
            SPI_M_IE &= ~UCRXIE;    //Disable RX Interrupt when finish.
        }
        else
        {
            *Spi_M_RxPtr++ = SPI_M_RXBUF;
            Spi_M_RxLen--;
        }
        break;
    case 4:     // Vector 4 - TXIFG
        break;
    default:
        break;
    }
}

/*****************************************************************************
 * [ SPI Master ] External Functions.
 *****************************************************************************/
void SpiMaster_init(unsigned long freq)
{
    // Set function IO.
    SET_SPI_M_FUNC_IO
    ;
    //**Reset**
    SPI_M_CTL1 |= UCSWRST;
    /* SPI MODE
     * Master Mode
     * 3-wire SPI
     * 1st edge sample
     * CLK rising edge sample
     * MSB first */
    SPI_M_CTL0 |= UCSYNC + UCMST + UCMODE_0 + 0 * UCCKPH + UCCKPL + UCMSB;

    // SMCLK source
    SPI_M_CTL1 |= UCSSEL__SMCLK;
    // Set clock divider
    unsigned int br = SMCLK_F / freq;
    SPI_M_BR0 = br & 0xFF;
    SPI_M_BR1 = br >> 8;
    //**Enable**
    SPI_M_CTL1 &= ~UCSWRST;
}

void SpiMaster_putc(char c)
{
    while (!(SPI_M_IFG & UCTXIFG))
    {
        ;
    }
    SPI_M_TXBUF = c;
}

void SpiMaster_puts(unsigned char *s, unsigned int len)
{
    //Transfer multiple byte.
    while (len--)
    {
        SpiMaster_putc(*s++);
    }
    //Wait SPI finish transfer .
    while (SPI_M_STAT & UCBUSY)
    {
        ;
    }
}

unsigned char SpiMaster_getc(void)
{
    return SPI_M_RXBUF;
}

void SpiMaster_gets(unsigned char *s, unsigned int len)
{
    Spi_M_RxPtr = s;
    Spi_M_RxLen = len;
    SPI_M_IFG &= ~UCRXIFG;
    SPI_M_IE |= UCRXIE;
}

void SpiMaster_setCsPins(unsigned int cs_sel)
{

#ifdef SET_SPI_M_CS_ALL_LOW     //Set CS_ALL
    if (cs_sel)
    {
        SET_SPI_M_CS_ALL_LOW;     //SET LOW
    }
    else
    {
        SET_SPI_M_CS_ALL_HIGH;     //GPIO HIGH
    }
#endif

#ifdef SPI_M_CS_GRP_L           //Set Low byte CS group ( CS_0~CS7 )
    unsigned char lowbyte = cs_sel & 0x00FF;
    if (lowbyte)
    {
        SPI_M_CS_GRP_L &= ~(SPI_M_CS_PIN_L & lowbyte);     //GPIO LOW
    }
    else
    {
        SPI_M_CS_GRP_L |= (SPI_M_CS_PIN_L & lowbyte);     //GPIO HIGH
    }
#endif

#ifdef SPI_M_CS_GRP_H           //Set High byte CS group ( CS_8~CS15 )
    unsigned char highbyte = (cs_sel >> 8) & 0x00FF;
    if (highbyte)
    {
        SPI_M_CS_GRP_H &= ~(SPI_M_CS_PIN_H & highbyte);     //GPIO LOW
    }
    else
    {
        SPI_M_CS_GRP_H |= (SPI_M_CS_PIN_H & highbyte);     //GPIO HIGH
    }
#endif
}
