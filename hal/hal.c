/**@file    hal.h
 *
 * MSP430F5xxx overall MCU HAL level code.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170106 | Yang.Zf | Initial Version
 */

#include "hal.h"
#include "board.h"

#ifndef SPI_MASTER_CLK
#define SPI_MASTER_CLK      1000000
#endif
#ifndef UART_BAUDRATE
#define UART_BAUDRATE       115200
#endif
#ifndef I2C_MASTER_CLK
#define I2C_MASTER_CLK      100000
#endif
#ifndef I2C_SLAVE_ADDRESS
#define I2C_SLAVE_ADDRESS   0x48
#endif

void Mcu_reset(void)
{
    /*MCU reset using soft POR*/
    PMMCTL0_H = PMMPW_H;
    PMMCTL0_L |= PMMSWPOR;
}

void Mcu_init(unsigned char bon)
{
    if (bon)
    {
        Clock_init();
        Uart_init(UART_BAUDRATE);
        SpiMaster_init(SPI_MASTER_CLK);
        SpiSlave_init();
        //I2cMaster_init(I2C_MASTER_CLK);
        I2cSlave_init(I2C_SLAVE_ADDRESS / 2);
        PwmOut_init();
        PwmIn_init();

        _EINT();
        /**************************************************************************************
         * SYSRSTIV is the Reset interrupt vector.
         * Generates a value that can be used as address offset for
         * fast interrupt service routine handling to identify the last cause of a reset
         * (BOR, POR, PUC) . Writing to this register clears all pending reset source flags.
         *
         * 00h = No interrupt pending
         * 02h = Brown out (BOR) (highest priority)
         * 04h = RST/NMI (BOR)
         * 06h = PMMSWBOR (BOR)
         * 08h = Wake up from LPMx.5 (BOR)
         * 0Ah = Security violation (BOR)
         * 0Ch = SVSL (POR)
         * 0Eh = SVSH (POR)
         * 10h = SVML_OVP (POR)
         * 12h = SVMH_OVP (POR)
         * 14h = PMMSWPOR (POR)
         * 16h = WDT time out (PUC)
         * 18h = WDT password violation (PUC)
         * 1Ah = Flash password violation (PUC)
         * 1Ch = Reserved
         * 1Eh = PERF peripheral/configuration area fetch (PUC)
         * 20h = PMM password violation (PUC)
         * 22h to 3Eh = Reserved
         **************************************************************************************/
        printf("\r\nBoard initialize finish, board name = [ %s ] , boot reason = [ 0x%x ].\r\n", BOARD_NAME, SYSRSTIV);
    }
    else
    {

    }
}

