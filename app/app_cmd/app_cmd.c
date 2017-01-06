/**@file    app_cmd.c
 *
 * UART & I2C control interface handler.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20161227 | Yang.Zf | Initial Version.
 */

#include "app.h"
#include "hal.h"
#include "string.h"

#define CMD_PRINT         printf

/*****************************************************************************
 * Internal Functions.
 *****************************************************************************/
uint16_t Cmd_Uart_gets(uint8_t *s)
{
    return Uart_gets(s);
}

uint16_t Cmd_Uart_getl(uint8_t *s)
{
    return Uart_getl(s);
}

void Cmd_Uart_clear(void)
{
    Uart_clear();
}

uint16_t Cmd_I2c_gets(uint8_t *s)
{
    return I2cSlave_gets(s);
}

void Cmd_I2c_puts(uint8_t *s, uint16_t len)
{
    I2cSlave_puts(s, len);
}

/*****************************************************************************
 * External Functions.
 *****************************************************************************/

uint8_t App_Cmd_handleI2c(void)
{

    return STATUS_SUCCESS;
}

uint8_t App_Cmd_handleUart(void)
{
    uint8_t cmd[256];
    uint8_t size;
    if (size = Uart_getl(cmd))
    {
        if(size == 1)
        {
            CMD_PRINT("\r\n");
        }
        else if(!memcmp(cmd,"test",4))
        {
            CMD_PRINT("\r\n Well received.");
        }
        else if(!memcmp(cmd,"player",6))
        {
            CMD_PRINT("\r\n Jump to PLAYER CMD");
        }
        else if(!memcmp(cmd,"plimit",6))
        {
            CMD_PRINT("\r\n Jump to PLIMIT CMD");
        }
        else
        {
            CMD_PRINT("\r\n CMD Error.");
        }

        Cmd_Uart_clear();
    }
    return STATUS_SUCCESS;
}
