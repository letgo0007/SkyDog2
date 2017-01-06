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
#include "hal.h"

/******************************************************************************
 * Operation define.
 *****************************************************************************/
#define PLAYER_SPI_S_MAX_SIZE       256
#define PLAYER_SPI_M_CS_SEL         0x01
#define PLAYER_SPI_M_CS_DATA_DELAY  100
#define PLAYER_SPI_M_DATA_CS_DELAY  100

#define PLAYER_DUTY_CH_MAX          256

/******************************************************************************
 * Operation buffers.
 *****************************************************************************/
static uint16_t gPlayer_DutyBuf[PLAYER_DUTY_CH_MAX];

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
    ;
}

/******************************************************************************
 * External Functions.
 *****************************************************************************/
uint16_t App_Player_getDuty(uint16_t *pu16duty, PLAYER_INPUT_MODEL emodel)
{
    if ((pu16duty == 0))
    {
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : NULL pointer.\r\n", __FUNCTION__);
        return PLAYER_FAIL;
    }

    uint16_t spi_size = 0;
    uint16_t duty_size = 0;
    uint8_t spi_buf[PLAYER_SPI_S_MAX_SIZE];
    //Get SPI Slave data.
    spi_size = Player_SpiSlave_gets(spi_buf);
    if (spi_size == 0)
    {
        return PLAYER_FAIL;
    }

    //Handle format convert.
    switch (emodel)
    {
    case IN_D8_P8:
    {
        duty_size = spi_size;
        Player_D8P8_TO_D12P16(spi_buf, pu16duty, spi_size);
        break;
    }
    case IN_D12_P8X1_5:
    {
        duty_size = spi_size * 2 / 3;
        Player_D12P8X1_5_TO_D12P16(spi_buf, pu16duty, spi_size);
        break;
    }
    case IN_D12_P8X2:
    {
        duty_size = spi_size / 2;
        Player_D12P8X2_TO_D12P16(spi_buf, pu16duty, spi_size);
        break;
    }
    default:
    {
        PLAYER_LOG("FUNC:[%s] emodel = %x , not supported format. ", __FUNCTION__, emodel);
        return 0;
    }
    }
    return duty_size;
}

PLAYER_RET App_Player_setDuty(uint16_t *pu16duty, uint16_t duty_size, PLAYER_OUTPUT_MODEL emodel)
{
    if ((pu16duty == 0))
    {
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : NULL pointer.\r\n", __FUNCTION__);
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
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : emodel = %x , not supported format. \r\n", __FUNCTION__, emodel);
        return PLAYER_FAIL;
    }
    }

    return PLAYER_SUCCESS;
}

PLAYER_RET App_Player_setTestPattern(PLAYER_TEST_PATTERN eptp, uint16_t duty_size, PLAYER_OUTPUT_MODEL emodel)
{
    switch (eptp)
    {
    case PTP_MUTE:
    {
        uint16_t i;
        uint16_t duty_buf[PLAYER_SPI_S_MAX_SIZE / 2];
        for (i = 0; i < duty_size; i++)
        {
            duty_buf[i] = 0x0000;
        }
        App_Player_setDuty(duty_buf, duty_size, emodel);
        break;
    }
    case PTP_MAX:
    {
        uint16_t i;
        uint16_t duty_buf[PLAYER_SPI_S_MAX_SIZE / 2];
        for (i = 0; i < duty_size; i++)
        {
            duty_buf[i] = 0x0FFF;
        }
        App_Player_setDuty(duty_buf, duty_size, emodel);
        break;
    }
    case PTP_50:
    {
        uint16_t i;
        uint16_t duty_buf[PLAYER_SPI_S_MAX_SIZE / 2];
        for (i = 0; i < duty_size; i++)
        {
            duty_buf[i] = 0x0800;
        }
        App_Player_setDuty(duty_buf, duty_size, emodel);
        break;
    }
    case PTP_RUN_HORSE:
    {
        uint16_t i;
        uint16_t duty_buf[PLAYER_SPI_S_MAX_SIZE / 2];
        for (i = 0; i < duty_size; i++)
        {
            duty_buf[i] = 0x0000;
        }

        static uint16_t frame_count = 0;
        static uint16_t light_index = 0;
        if (frame_count >= 60)
        {
            duty_buf[light_index] = 0x0800;
            duty_buf[light_index - 1] = 0x0000;
            light_index++;
            frame_count = 0;
        }

        App_Player_setDuty(duty_buf, duty_size, emodel);
        break;
    }
    default:
    {
        PLAYER_LOG("\r\nFUNC:[%s] ERROR : eptp = %x , not supported index. \r\n", __FUNCTION__, emodel);
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
        PLAYER_LOG("\r\nFUNC:[%s] WARNING : Invalid Pointer , Print Default Duty Buffer.\r\n", __FUNCTION__);
        pu16duty = gPlayer_DutyBuf;
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
    uint16_t duty_size;

    duty_size = App_Player_getDuty(gPlayer_DutyBuf, param->pin_model);
    App_Player_setDuty(gPlayer_DutyBuf, duty_size, param->pout_model);

    return PLAYER_SUCCESS;
}
