/**@file    board.c
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#include "board.h"
#include "hal.h"

void Board_init(unsigned char bon)
{
    if (bon) //Turn on board.
    {
        Clock_init();
        Uart_init(115200);
        SpiMaster_init(1000000);
        SpiSlave_init();
        I2cSlave_init(0x48/2);
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
    else //Turn off board.
    {

    }

}

