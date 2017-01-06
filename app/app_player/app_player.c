/**@file    app_player.c
 *
 * Local Dimming LED module display player function .
 * 1 Get Duty from SPI.
 * 2 Set Duty to BL.
 *
 * With 3 build-in SPI Raw data receiving / transmitting function.
 * Custom device need external BSP support ( e.g. IW7027 ).
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170106 | Yang.Zf | Initial Version
 */

#include "app_player.h"
#include "app.h"
#include "bsp.h"
#include "hal.h"

/******************************************************************************
 * Operation define.
 *****************************************************************************/
#define PLAYER_SPI_S_MAX_SIZE       256
#define PLAYER_SPI_M_CS_SEL         0x01
#define PLAYER_SPI_M_CS_DATA_DELAY  100
#define PLAYER_SPI_M_DATA_CS_DELAY  100

#define PLAYER_DUTY_CH_MAX          128

/******************************************************************************
 * Internal Variables.
 *****************************************************************************/
uint16_t gPlayer_LdDutyBuf[PLAYER_DUTY_CH_MAX];     //Local Dimming duty buffer.
uint16_t gPlayer_TpDutyBuf[PLAYER_DUTY_CH_MAX];     //Test Pattern duty buffer.

/******************************************************************************
 * Internal Functions.
 *****************************************************************************/
#define PLAYER_LOG                  printf
#define PLAYER_DELAYUS              DELAY_US

uint16_t Player_D8P8_TO_D12P16(uint8_t* pu8in, uint16_t *pu16out, uint16_t in_size)
{
    uint16_t i;
    for (i = 0; i < in_size; i++)
    {
        //0xAB -> 0x0AB0
        pu16out[i] = ((uint16_t) pu8in[i]) << 4;
    }

    return i;
}

uint16_t Player_D12P8X1_5_TO_D12P16(uint8_t* pu8in, uint16_t *pu16out, uint16_t in_size)
{
    uint16_t i;
    uint16_t j = 0;

    for (i = 0; i < in_size * 2 / 3;)
    {
        //0xAB , 0xCD , 0xEF -> 0x0ABC , 0x0DEF
        pu16out[i] = ((uint16_t) pu8in[j] << 4) + (pu8in[j + 1] >> 4);
        pu16out[i + 1] = ((uint16_t) (pu8in[j + 1] & 0x0F) << 8) + pu8in[j + 2];
        i = i + 2;
        j = j + 3;
    }

    return i;
}

uint16_t Player_D12P8X2_TO_D12P16(uint8_t* pu8in, uint16_t *pu16out, uint16_t in_size)
{
    uint16_t i;
    uint16_t j = 0;

    for (i = 0; i < in_size / 2; i++)
    {
        //0x0A , 0xBC -> 0x0ABC
        pu16out[i] = ((uint16_t) pu8in[j++] << 8) + pu8in[j++];
    }

    return i;
}

uint16_t Player_D12P16_TO_D8P8(uint16_t *pu16in, uint8_t* pu8out, uint16_t in_size)
{
    uint16_t i;
    for (i = 0; i < in_size; i++)
    {
        // 0x0ABC -> 0xAB
        pu8out[i] = pu16in[i] >> 4;
    }

    return i;
}

uint16_t Player_D12P16_TO_D8P8X1_5(uint16_t *pu16in, uint8_t* pu8out, uint16_t in_size)
{
    uint16_t i;
    uint16_t j = 0;
    for (i = 0; i < in_size; i++)
    {
        // 0x0ABC 0x0DEF -> 0xAB, 0xCD, 0xEF
        pu8out[j++] = pu16in[i] >> 4;
        pu8out[j++] = ((pu16in[i] & 0x0F) << 4) + ((pu16in[i + 1] & 0x0F00) >> 8);
        pu8out[j++] = (pu16in[i + 1]) & 0xFF;
    }

    return j;
}

uint16_t Player_D12P16_TO_D8P8X2(uint16_t *pu16in, uint8_t* pu8out, uint16_t in_size)
{
    uint16_t i;
    uint16_t j = 0;
    for (i = 0; i < in_size; i++)
    {
        // 0x0ABC -> 0x0A , 0xBC
        pu8out[j++] = (pu16in[i] & 0x0F00) >> 8;
        pu8out[j++] = (pu16in[i]) & 0xFF;
    }

    return j;
}

uint16_t Player_SpiSlave_gets(uint8_t* s)
{
    uint16_t size = 0;
    size = SpiSlave_gets(s);
    if (size)
    {
        SpiSlave_clear();
    }
    return size;
}

void Player_SpiMaster_puts(uint8_t* s, uint16_t len)
{
    SpiMaster_setCsPins(PLAYER_SPI_M_CS_SEL);
    PLAYER_DELAYUS(PLAYER_SPI_M_CS_DATA_DELAY);
    SpiMaster_puts(s, len);
    PLAYER_DELAYUS(PLAYER_SPI_M_DATA_CS_DELAY);
    SpiMaster_setCsPins(0);
}

void Player_Cpld_setDuty(uint16_t *pu16duty, uint16_t duty_size, PLAYER_OUTPUT_MODEL emodel)
{
    //Add 1 12bit tail to the end of duty.
    pu16duty[duty_size] = 0xDD0;

    // Convert Data to Duty = 12bit , package = 8bit X 1.5 .
    uint8_t spi_buf[PLAYER_SPI_S_MAX_SIZE];
    uint16_t spi_size;
    spi_size = Player_D12P16_TO_D8P8X1_5(pu16duty, spi_buf, duty_size + 1);

    // Send Data
    Player_SpiMaster_puts(spi_buf, spi_size);
}

void Player_Iw7027_setDuty(uint16_t *pu16duty, uint16_t duty_size, PLAYER_OUTPUT_MODEL emodel)
{
    Iw7027_setDuty(pu16duty, 0);
}

uint16_t Player_VsyncIn_getFreq(void)
{
    return PwmIn_getFreq(1);
}

uint16_t Player_VsyncIn_getRiseEdgeFlag(void)
{
    return PwmIn_getRiseEdgeFlag(1);
}

void Player_VsyncOut_setPwm(uint16_t freq, uint16_t duty)
{
    PwmOut_setOutput(2, freq, duty);
}

uint16_t Player_VsyncOut_getRiseEdgeFlag()
{
    return PwmOut_getRiseEdgeFlag(2);
}

/******************************************************************************
 * External Functions.
 *****************************************************************************/
uint16_t App_Player_getDuty(uint16_t *pu16duty, PLAYER_INPUT_MODEL emodel)
{
    //Check Parameter Validation.
    if (emodel == IN_DISABLE)
    {
        return 0;
    }
    if ((pu16duty == 0))
    {
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : NULL pointer.", __FUNCTION__);
        return 0;
    }
    uint16_t spi_size = 0;
    uint16_t duty_size = 0;
    uint8_t spi_buf[PLAYER_SPI_S_MAX_SIZE];

    //Get data from SPI Slave.
    spi_size = Player_SpiSlave_gets(spi_buf);
    if (spi_size == 0)
    {
        return 0;
    }

    //Handle format convert.
    switch (emodel)
    {
    case IN_D8_P8:
    {
        duty_size = Player_D8P8_TO_D12P16(spi_buf, pu16duty, spi_size);
        break;
    }
    case IN_D12_P8X1_5:
    {
        duty_size = Player_D12P8X1_5_TO_D12P16(spi_buf, pu16duty, spi_size);
        break;
    }
    case IN_D12_P8X2:
    {
        duty_size = Player_D12P8X2_TO_D12P16(spi_buf, pu16duty, spi_size);
        break;
    }
    case IN_MFC11_SU860A_6X10:
    {
        /* YZF : D0 + D1 + ... + DN + TAIL. Data = 12bit.
         *       Don't know the exact format of TAIL , but it seems not cared...
         *       Just ignore the TAIL , get fixed amount of duty according to model index.
         */
        duty_size = 60;
        Player_D12P8X1_5_TO_D12P16(spi_buf, pu16duty, spi_size);
        break;
    }
    case IN_MFC11_SU860A_6X13:
    {
        duty_size = 78;
        Player_D12P8X1_5_TO_D12P16(spi_buf, pu16duty, spi_size);
        break;
    }
    default:
    {
        PLAYER_LOG("\r\nFUNC:[%s] emodel = %x , not supported format.", __FUNCTION__, emodel);
        return 0;
    }
    }
    return duty_size;
}

PLAYER_RET App_Player_setDuty(uint16_t *pu16duty, uint16_t duty_size, PLAYER_OUTPUT_MODEL emodel)
{
    if (emodel == OUT_DISABLE)
    {
        return PLAYER_FAIL;
    }

    if ((pu16duty == 0))
    {
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : NULL pointer.", __FUNCTION__);
        return PLAYER_FAIL;
    }

    switch (emodel)
    {
    case OUT_D8_P8:
    {
        uint8_t spi_buf[PLAYER_SPI_S_MAX_SIZE];                 //SPI Master sending buffer.
        Player_D12P16_TO_D8P8(pu16duty, spi_buf, duty_size);    //Convert data format.
        Player_SpiMaster_puts(spi_buf, duty_size);              //SPI out.

        break;
    }
    case OUT_D12_P8X1_5:
    {
        uint8_t spi_buf[PLAYER_SPI_S_MAX_SIZE];                     //SPI Master sending buffer.
        Player_D12P16_TO_D8P8X1_5(pu16duty, spi_buf, duty_size);    //Convert data format.
        Player_SpiMaster_puts(spi_buf, duty_size * 3 / 2);          //SPI out.
        break;
    }
    case OUT_D12_P8X2:
    {
        uint8_t spi_buf[PLAYER_SPI_S_MAX_SIZE];                 //SPI Master sending buffer.
        Player_D12P16_TO_D8P8X2(pu16duty, spi_buf, duty_size);  //Convert data format.
        Player_SpiMaster_puts(spi_buf, duty_size * 2);          //SPI out.
        break;
    }
    case OUT_IW7027_GOA_16X1:
    case OUT_IW7027_SU860A_6X10:
    case OUT_IW7027_SU860A_6X13:
    {
        Player_Iw7027_setDuty(pu16duty, duty_size, emodel);     //Hand over SPI Master to BSP
        break;
    }
    case OUT_CPLD_SU860A_6X10:
    case OUT_CPLD_SU860A_6X13:
    {
        Player_Cpld_setDuty(pu16duty, duty_size, emodel);       //Hand over SPI Master to BSP
        break;
    }
    default:
    {
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : emodel = %x , not supported format.", __FUNCTION__, emodel);
        return PLAYER_FAIL;
    }
    }
    return PLAYER_SUCCESS;
}

PLAYER_RET App_Player_prepareTestPattern(uint16_t *pu16tpbuf, uint16_t duty_size, PLAYER_TEST_PATTERN eptp)
{
    switch (eptp)
    {
    case PTP_MUTE:
    {
        uint16_t i;
        for (i = 0; i < duty_size; i++)
        {
            pu16tpbuf[i] = 0x0000;
        }
        break;
    }
    case PTP_MAX:
    {
        uint16_t i;
        for (i = 0; i < duty_size; i++)
        {
            pu16tpbuf[i] = 0x0FFF;
        }
        break;
    }
    case PTP_50:
    {
        uint16_t i;
        for (i = 0; i < duty_size; i++)
        {
            pu16tpbuf[i] = 0x0800;
        }
        break;
    }
    case PTP_RUN_HORSE:
    {
        uint16_t i;
        static uint16_t frame_count = 60;
        static uint16_t light_index = 0;

        //Run Horse every 60Frame.
        frame_count++;
        if (frame_count >= 60)
        {
            //Build Black Screen
            for (i = 0; i < duty_size; i++)
            {
                pu16tpbuf[i] = 0x0000;
            }
            //Light Up 1 channel.
            pu16tpbuf[light_index] = 0x0800;
            light_index++;
            //Reset count & handle over range.
            frame_count = 0;
            if (light_index >= duty_size)
            {
                light_index = 0;
            }
        }
        break;
    }
    default:
    {
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : eptp = %x , not supported index.", __FUNCTION__, eptp);
        return PLAYER_FAIL;
    }
    }

    return PLAYER_SUCCESS;
}

PLAYER_RET App_Player_printDuty(uint16_t *pu16duty, uint16_t col, uint16_t row)
{
    if ((pu16duty == 0))
    {
        //If pointer is null , print default duty buffer .
        PLAYER_LOG("\r\nFUNC:[%s] WARNING : Invalid Pointer , Print Default Duty Buffer.", __FUNCTION__);
        pu16duty = gPlayer_LdDutyBuf;
    }

    PLAYER_LOG("\r\nPrint Duty , COL = [%d] , ROW = [%d].\r\n", col, row);

    uint16_t i;
    for (i = 0; i < col * row; i++)
    {
        if (i % col == 0)
        {
            PLAYER_LOG("\r\n");
        }
        PLAYER_LOG("\t%x ", pu16duty[i]);
    }
    PLAYER_LOG("\r\n");
    return PLAYER_SUCCESS;
}

PLAYER_RET App_Player_setWorkParam(PLAYER_PARAM *param)
{
    return PLAYER_SUCCESS;
}

PLAYER_RET App_Player(PLAYER_PARAM *param)
{
    /*Step 1 : Get Duty from SPI slave .*/
    uint16_t spi_duty_size;
    spi_duty_size = App_Player_getDuty(gPlayer_LdDutyBuf, param->pin_model);

    /*Step 2 : Decide Duty output size according to SYNC MODE. */
    uint16_t output_duty_size = 0;

    switch (param->psync_mode)
    {
    case PSYNC_OUT_ON_SPI_IN:
    {
        if (spi_duty_size)   //Output when SPI input is valid.
        {
            output_duty_size = spi_duty_size;
        }
        break;
    }
    case PSYNC_OUT_ON_VSYNC_IN:
    {
        if (Player_VsyncOut_getRiseEdgeFlag())
        {
            output_duty_size = param->pch_amount;
        }
        break;
    }
    case PSYNC_OUT_ON_VSYNC_OUT:
    {
        if (Player_VsyncOut_getRiseEdgeFlag())
        {
            output_duty_size = param->pch_amount;
        }
        break;
    }
    case PSYNC_OUT_ON_INTERNAL_60Hz:
    {
        break;
    }
    default:
    {
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : [param->psync_mode] == not supported sync mode.", __FUNCTION__);
        break;
    }
    }

    //Step 3: Duty send out when output_duty_size != 0
    if (output_duty_size)
    {
        if (param->ptest_pattern != PTP_DISABLE)   //Test Mode
        {
            //Prepare Test Pattern
            App_Player_prepareTestPattern(gPlayer_TpDutyBuf, param->pch_amount, param->ptest_pattern);

            //Send Test Pattern
            App_Player_setDuty(gPlayer_TpDutyBuf, param->pch_amount, param->pout_model);
        }
        else
        {
            //Send Local Dimming Duty.
            App_Player_setDuty(gPlayer_LdDutyBuf, param->pch_amount, param->pout_model);
        }
    }

    return PLAYER_SUCCESS;
}
