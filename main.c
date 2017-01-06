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
        WATCHDOG_FEED;

        App_Cmd_Uart();
        App_Cmd_I2c();

        App_Player(&gPlayerParam);

        DELAY_MS(10);
    }
}
