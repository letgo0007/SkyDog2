#include <msp430.h> 
#include <stdio.h>
#include "hal.h"
#include "bsp.h"
#include "app.h"
#include "board.h"

PLAYER_PARAM gPlayerParam =
{
        .pch_amount = 60,
        .pin_model = IN_D8_P8,
        .pout_model = OUT_D8_P8,
        .psync_mode = PSYNC_OUT_ON_VSYNC_OUT,
        .ptest_pattern =PTP_RUN_HORSE,
};
uint8_t buf[10];
//This is a git test

//Here is something more.

int main(void)
{
    WATCHDOG_FEED;
    Mcu_init(ON);
    APP_PLIMIT_init(1);

    //Test ONLY
    PwmOut_setOutput(1, 60, 128);
    PwmOut_setOutput(2, 60, 64);

    while (1)
    {
        WATCHDOG_HOLD;

        App_Cmd_Uart();
        App_Cmd_I2c();

        uint8_t result;

        //result = I2cMaster_write(0x48,"ABCDEFG",1);

        //printf("\r\n I2C write result 0x%x",result);

        result = I2cMaster_wread(0x48,"ABCDEFG",7,buf,1);

        printf("\r\n I2C read result 0x%x",result);

        //App_Player(&gPlayerParam);

        DELAY_MS(10);
    }
}
