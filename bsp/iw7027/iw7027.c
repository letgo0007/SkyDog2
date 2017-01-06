/**@file    iw7027.c
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 2017Äê1ÔÂ6ÈÕ | GKX100052 | Initial Version
 */
#include "iw7027.h"
#include "hal.h"

/*****************************************************************************
 * Operation Constant Define
 *****************************************************************************/
#define IW7027_SPIM_CS_TO_DATA_DELAY    100
#define IW7027_SPIM_DATA_TO_CS_DELAY    100
#define IW7027_SPIM_MAX_BUF_SIZE        256
#define IW7027_SPIM_READ_CHECK_RETRY    10
#define IW7027_SPIM_READ_CHECK_DELAY    100

#define IW7027_PWM_OUT_CH               1

/*****************************************************************************
 * Internal Functions.
 *****************************************************************************/

#define IW7027_LOG          printf
#define IW7027_DELAY_US     DELAY_US
#define IW7027_DELAY_MS     DELAY_MS

uint16_t IW_D12P16_TO_D8P8X2(uint16_t *pu16in, uint8_t* pu8out, uint16_t in_size)
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

void IwSpiMaster_putc(uint8_t c)
{
    SpiMaster_putc(c);
}

uint8_t IwSpiMaster_getc(void)
{
    return SpiMaster_getc();
}

void IwSpiMaster_puts(uint8_t *s, uint16_t len)
{
    SpiMaster_puts(s, len);
}

void IwSpiMaster_gets(uint8_t *s, uint16_t len)
{
    SpiMaster_gets(s, len);
}

void IwSpiMaster_setCsPins(uint16_t cs)
{
    if (cs)
    {
        SpiMaster_setCsPins(cs);
        IW7027_DELAY_US(IW7027_SPIM_CS_TO_DATA_DELAY);
    }
    else
    {
        IW7027_DELAY_US(IW7027_SPIM_DATA_TO_CS_DELAY);
        SpiMaster_setCsPins(cs);
    }
}

void IwVsyncOut_setOutput(uint16_t freq, uint16_t duty)
{
    PwmOut_setOutput(IW7027_PWM_OUT_CH, freq, duty * 256 / 100);
}

/*****************************************************************************
 * External Functions.
 *****************************************************************************/

void Iw7027_putc(uint16_t iw_sel, uint8_t reg, uint8_t u8data)
{
#if IW7027_DAISY_CHAIN  //Daisy-Chain Mode SPI

#else //Parallel Mode SPI
    //Set CS pins.
    IwSpiMaster_setCsPins(iw_sel);

    /* Prepare SPI transmit buffer.
     * Reference : <Dialog TV BL driver_SPI_Interface.pdf> P10 <3.1. Write Single Data>
     * */
    uint8_t spi_buf[3];
    spi_buf[0] = 0xC0;
    spi_buf[1] = reg;
    spi_buf[2] = u8data;

    // SPI Transmit
    IwSpiMaster_puts(spi_buf, 3);

    //Reset CS pins.
    IwSpiMaster_setCsPins(0x00);
#endif
}

uint8_t Iw7027_getc(uint16_t iw_sel, uint8_t reg)
{
#if IW7027_DAISY_CHAIN  //Daisy-Chain Mode SPI

#else //Parallel Mode SPI
    //Set CS pins.
    IwSpiMaster_setCsPins(iw_sel);

    /* Prepare SPI transmit buffer.
     * Reference : <Dialog TV BL driver_SPI_Interface.pdf> P10 <3.1. Write Single Data>
     * */
    uint8_t spi_buf[5];
    spi_buf[0] = 0x41;
    spi_buf[1] = reg | 0x80;
    spi_buf[2] = 0x00;
    spi_buf[3] = 0x00;
    spi_buf[4] = 0x00;

    // SPI Transmit
    IwSpiMaster_puts(spi_buf, 5);
    uint8_t ret = IwSpiMaster_getc();

    //Reset CS pins.
    IwSpiMaster_setCsPins(0x00);

    return ret;
#endif
}

IW7027_RET Iw7027_puts(uint16_t iw_sel, uint8_t reg, uint8_t len, uint8_t *pu8data)
{
#if IW7027_DAISY_CHAIN  //Daisy-Chain Mode SPI

#else //Parallel Mode SPI
    //Set CS pins.
    IwSpiMaster_setCsPins(iw_sel);

    /* Prepare SPI transmit buffer.
     * Reference : <Dialog TV BL driver_SPI_Interface.pdf> P10 <3.1. Write Single Data>
     * */
    uint8_t spi_head[3];
    spi_head[0] = 0x01;
    spi_head[1] = len;
    spi_head[2] = reg;

    // SPI Transmit Head + Data
    IwSpiMaster_puts(spi_head, 3);
    IwSpiMaster_puts(pu8data, len);

    //Reset CS pins.
    IwSpiMaster_setCsPins(0x00);

    return IW7027_SUCCESS;
#endif
}

IW7027_RET Iw7027_gets(uint16_t iw_sel, uint8_t reg, uint8_t len, uint8_t *pu8data)
{
#if IW7027_DAISY_CHAIN  //Daisy-Chain Mode SPI

#else //Parallel Mode SPI

    //Not Available YET

    return IW7027_FAIL;
#endif
}

IW7027_RET Iw7027_checkGetWithTimeOut(uint16_t iw_sel, uint8_t reg, uint8_t compare_val, uint8_t compare_mask)
{
    uint8_t chipcount;
    uint8_t retrycount;
    uint8_t status;
    uint8_t val;

    uint16_t i;
    for (i = 0; i < IW7027_DEV_AMOUNT; i++) // Check from IW_0 to IW_N
    {
        if (iw_sel & IW_SEL_LIST[i]) //Only check selected device.
        {
            retrycount = IW7027_SPIM_READ_CHECK_RETRY;
            status = 0x01;

            //Special operation when access register address of 0x80~0xFF
            //Reference: <iW7037 SPI Mode Application Notes V01.pdf> Page30 <How to read the status registers from [0x80] to [0xFF]>
            //Unlock READ_EXTD.
            if (reg >= 0x80)
            {
                Iw7027_putc((iw_sel & chipcount), 0x78, 0x80);
            }

            //Read & check result with retry.
            while (--retrycount && status)
            {
                val = Iw7027_getc((iw_sel & chipcount), reg);

                if ((val & compare_mask) == compare_val)
                {
                    //Value check correct , set status = 0 to end the loop.
                    status = 0;
                }
                else
                {
                    //Value check incorrect , delay between retry.
                    IW7027_DELAY_MS(IW7027_SPIM_READ_CHECK_DELAY);
                }
            }

            //Lock READ_EXTD when finish.
            if (reg >= 0x80)
            {
                Iw7027_putc((iw_sel & chipcount), 0x78, 0x00);
            }
            if (retrycount == 0)
            {
                //if time out , return fail.
                return IW7027_FAIL;
            }
        }
    }

    //If Check loop finish without return fail ,return success .
    return IW7027_SUCCESS;
}

IW7027_RET Iw7027_init(uint8_t bon, uint8_t *init_map)
{
    IW7027_LOG("\r\nIW7027 Initial Start , Operation code = %x", bon);

    if (bon)
    {
        //STEP 1 : Force Reset all IW7027 devices.
        IwVsyncOut_setOutput(0, 0);              //Disable VSYNC OUT.
        Iw7027_putc(IW_SEL_ALL, 0x00, 0x00);     //Reset IW7027
        IW7027_DELAY_MS(100);

        //STEP 2 : Read IW7027 Chip ID.
        //        This step is optional.
        if (IW7027_FAIL == Iw7027_checkGetWithTimeOut(IW_SEL_ALL, 0xEB, 0x24, 0xFF))
        {
            IW7027_LOG("\r\nERROR : IW7027 Chip ID Check Fail!!!");
        }

        //STEP 3 : Write Initialize Table.
        if (init_map)
        {
            uint16_t i;
            for (i = 0; i < IW7027_DEV_AMOUNT; i++)
            {
                Iw7027_puts(IW_SEL_LIST[i], 0x00, 0x60, &init_map[0x60 * i]);
            }
        }

        //STEP 4 : Turn on VSYNC & set working frequency.
        IwVsyncOut_setOutput(60, 50);
        IW7027_DELAY_MS(100);
        Iw7027_setFreq(60, 1);

        //STEP 5 : Initialize finish ,turn on BL.
        Iw7027_putc(IW_SEL_ALL, 0x00, 0x05);
    }
    else
    {
        Iw7027_putc(IW_SEL_ALL, 0x00, 0x00);     //Reset IW7027
    }

    IW7027_LOG("\r\nIW7027 Initial Finish , Operation code = %x", bon);
    return IW7027_SUCCESS;
}

IW7027_RET Iw7027_setDuty(uint16_t *duty, uint8_t *sort_map)
{
    uint16_t i;
    uint8_t spi_buf[32 * IW7027_DEV_AMOUNT];

    // Prepare SPI send data.
    if (sort_map == 0)
    {
        //No need sorting when sort map = NULL , do data convert only.
        IW_D12P16_TO_D8P8X2(duty, spi_buf, IW_CH_AMOUNT);
    }
    else
    {
        //Do data convert with re-sorting order of data.
        for (i = 0; i < IW_CH_AMOUNT; i++)
        {
            spi_buf[2 * sort_map[i]] = duty[i] >> 8;
            spi_buf[2 * sort_map[i] + 1] = duty[i] & 0xFF;
        }
    }

    // SPI Data sending from IW_0 to IW_N
    for (i = 0; i < IW7027_DEV_AMOUNT; i++)
    {
        Iw7027_puts(IW_SEL_LIST[i], 0x40, 32, spi_buf + 32 * i);
    }
    return IW7027_SUCCESS;
}

IW7027_RET Iw7027_setCurrent(uint8_t current)
{
    IW7027_RET status;

    //Write data to IW7037
    //1 . Disable Protect , Set [FAUL_LOCK] (0x62  BIT0)to 1
    Iw7027_putc( IW_SEL_ALL, 0x62, 0x01);

    //2 . Write current to 0x27
    Iw7027_putc( IW_SEL_ALL, 0x27, current);

    //3 . Check status. Low 4 bit of 0xB3 = 0x05
    status = Iw7027_checkGetWithTimeOut( IW_SEL_ALL, 0xB3, 0x05, 0x0F);

    //4 . Enable Protect , Set [FAULT LOCK] (0x62  BIT0)to 0 ,IDAC_REMAP + FAUL_LOCK
    Iw7027_putc( IW_SEL_ALL, 0x62, 0x00);

    return status;
}

IW7027_RET Iw7027_setFreq(uint8_t freq, uint8_t n);
