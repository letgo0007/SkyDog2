#include <msp430.h> 
#include <stdio.h>
#include "hal.h"
#include "bsp.h"
#include "app.h"
#include "board.h"

/*
 * main.c
 */
unsigned int rxbuf[256];

int main(void)
{
    WATCHDOG_FEED;
    Board_init(ON);

    PwmOut_setOutput(0, 60, 0);
    PwmOut_setOutput(1, 60, 128);
    PwmOut_setOutput(2, 60, 64);

    while (1)
    {
        WATCHDOG_FEED;
        unsigned char size;

        App_Cmd_handleI2c();
        App_Cmd_handleUart();

#if 1
        //SpiSlave_puts("ABCDEFGHIJ", 10);
        //SpiMaster_gets(&rxbuf[100], 5);

        SpiMaster_setCsPins(1);
        SpiMaster_puts("0123456789", 10);
        SpiMaster_setCsPins(0);

        size = App_Player_getDuty(rxbuf, IN_D8_P8);
        if (size)
        {
            printf("\r\nGet SPI data number %d ", size);
            App_Player_printDuty(rxbuf, 5, 4);
            App_Player_setDuty(rxbuf, 5, OUT_CPLD_SU860A_6X10);
        }
#endif
        DELAY_MS(100);
    }
}
