/******************************************************************************
 * @file    app_plimit_cmd.c
 *
 * I2C command interface of PLIMIT function.
 *
 * @pre     Need API to get I2C command data.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20161212 | Yang.Zf | Initial Version
 *****************************************************************************/

#include "app_plimit_cmd.h"

/***Internal Defines**********************************************************/

//I2C Command maximum data length
#define CMD_DATA_MAX        256

//I2C Command Format.
typedef struct tPlimitCmd
{
    HI_U8 u8MainCmd;                  // Checksum & Main Command ID
    HI_U8 u8SubCmd;                   // Sub Command ID
    HI_U8 au8Param[CMD_DATA_MAX];     // Parameter
} tPlimitCmd;

/***External Functions********************************************************/

//I2C Slave Write Interface
PLIMIT_RET Plimit_Cmd_WriteI2CSlaveBuf(HI_U8 *pu8data, HI_U8 length)
{
#ifdef PLIMIT_LIB_USE_ON_MSP430
    //TBD.
#endif
#ifdef PLIMIT_LIB_USE_ON_HISILICON
    //HI_DRV_I2C_S_Write(HI_DRV_I2C_S_ADDR_ID_0, pu8data, length );
#endif
    return PLIMIT_SUCCESS;
}

/***External Functions********************************************************/
PLIMIT_RET App_Plimit_Cmd(HI_U8 *pu8cmd, tPlimit_Data *pstdata, tPlimit_Param *pstparam, tPlimit_Db *pstdb)
{
    PLIMIT_CKECK_NULL_POINTER(pu8cmd);
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    //Link command to I2C received data.
    tPlimitCmd *pstcmd;
    pstcmd = (tPlimitCmd *) pu8cmd;

    //Switch function by SubCmd
    switch (pstcmd->u8SubCmd)
    {
    case CUS_PLIMIT_SET_ENABLE:
    {
        pstparam->PlimitEn = pstcmd->au8Param[0];
        //Param[1] : Set Gamma Table.
        if (pstcmd->au8Param[1] == 0x01)
        {
            App_Plimit_setGammaTable(pstdata, pstparam);
        }
        break;
    }
    case CUS_PLIMIT_GET_ENABLE:
    {
        HI_U8 buf[2];

        buf[0] = pstparam->PlimitEn;
        buf[1] = 0x00;

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));

        break;
    }
    case CUS_PLIMIT_SET_PARAM_INDEX:
    {
        App_Plimit_Db_setParamIndex(pstcmd->au8Param[0], pstparam, pstdb);
        break;
    }
    case CUS_PLIMIT_GET_PARAM_INDEX:
    {
        HI_U8 buf[1];

        App_Plimit_Db_getParamIndex(buf, pstdb);

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_SET_SAMPLE_RATE:
    {
        pstparam->FrameRate = pstcmd->au8Param[0];
        pstparam->FramePerSample = pstcmd->au8Param[1];
        break;
    }
    case CUS_PLIMIT_GET_SAMPLE_RATE:
    {
        HI_U8 buf[2];

        buf[0] = pstparam->FrameRate;
        buf[1] = pstparam->FramePerSample;

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_SET_BL_GAMMA_POINT:
    {
        pstparam->GammaTable[0x200 * pstcmd->au8Param[0]] = (HI_U16) pstcmd->au8Param[1] * 0x100 + pstcmd->au8Param[2];
        break;
    }
    case CUS_PLIMIT_GET_BL_GAMMA_POINT:
    {
        HI_U8 buf[3];

        buf[0] = pstcmd->au8Param[0];
        buf[1] = pstparam->GammaTable[0x200 * pstcmd->au8Param[0]] >> 8;
        buf[2] = pstparam->GammaTable[0x200 * pstcmd->au8Param[0]] & 0xFF;

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_SET_DUTY_PARAM:
    {
        pstparam->GlobalCurrent = pstcmd->au8Param[0];
        pstparam->GlobalDutyMax = (HI_U16) pstcmd->au8Param[1] * 0x100 + pstcmd->au8Param[2];
        pstparam->LocalDutyMax = (HI_U16) pstcmd->au8Param[3] * 0x100 + pstcmd->au8Param[4];
        break;
    }
    case CUS_PLIMIT_GET_DUTY_PARAM:
    {
        HI_U8 buf[5];

        buf[0] = pstparam->GlobalCurrent;
        buf[1] = pstparam->GlobalDutyMax >> 8;
        buf[2] = pstparam->GlobalDutyMax & 0xFF;
        buf[3] = pstparam->LocalDutyMax >> 8;
        buf[4] = pstparam->LocalDutyMax & 0xFF;

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_SET_TEMP_COEF:
    {
        pstparam->CoefD = (HI_U16) pstcmd->au8Param[0] * 0x100 + pstcmd->au8Param[1];
        pstparam->CoefR = (HI_U16) pstcmd->au8Param[2] * 0x100 + pstcmd->au8Param[3];
        break;
    }
    case CUS_PLIMIT_GET_TEMP_COEF:
    {
        HI_U8 buf[4];

        buf[0] = pstparam->CoefD >> 8;
        buf[1] = pstparam->CoefD & 0xFF;
        buf[2] = pstparam->CoefR >> 8;
        buf[3] = pstparam->CoefR & 0xFF;

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_SET_TEMP_THLD:
    {
        pstparam->TempHighGap = pstcmd->au8Param[0];
        pstparam->TempLowGap = pstcmd->au8Param[1];
        break;
    }
    case CUS_PLIMIT_GET_TEMP_THLD:
    {
        HI_U8 buf[2];

        buf[0] = pstparam->TempHighGap;
        buf[1] = pstparam->TempLowGap;

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_SET_TEMP_STEP:
    {
        pstparam->StepLimitUp = (HI_U16) pstcmd->au8Param[0] * 0x100 + pstcmd->au8Param[1];
        pstparam->StepLimitDown = (HI_U16) pstcmd->au8Param[2] * 0x100 + pstcmd->au8Param[3];
        break;
    }
    case CUS_PLIMIT_GET_TEMP_STEP:
    {
        HI_U8 buf[5];

        buf[0] = pstparam->StepLimitUp >> 8;
        buf[1] = pstparam->StepLimitUp & 0xFF;
        buf[2] = pstparam->StepLimitDown >> 8;
        buf[3] = pstparam->StepLimitDown & 0xFF;

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_SET_TEMP_SAFE_DUTY:
    {
        HI_U16 duty, id, i;

        id = pstcmd->au8Param[0];
        duty = (HI_U16) pstcmd->au8Param[1] * 0x100 + pstcmd->au8Param[2];

        if (id == 0xFF)
        {
            //Set all channels.
            for (i = 0; i < pstparam->ChAmount; i++)
            {
                pstparam->DutyTempSafePtr[i] = duty;
            }
        }
        else
        {
            pstparam->DutyTempSafePtr[id] = duty;
        }

        break;
    }
    case CUS_PLIMIT_GET_TEMP_SAFE_DUTY:
    {
        HI_U8 buf[3];

        buf[0] = pstcmd->au8Param[0];
        buf[1] = pstparam->DutyTempSafePtr[buf[0]] >> 8;
        buf[2] = pstparam->DutyTempSafePtr[buf[0]] & 0xFF;

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_SET_ENV_TEMP:
    {
        HI_U16 i;
        if (pstcmd->au8Param[0] == 0xFF)
        {
            //Set all channels.
            for (i = 0; i < pstparam->ChAmount; i++)
            {
                pstdata->TempSafe[i] = pstcmd->au8Param[1];
            }
        }
        else
        {
            pstdata->TempSafe[pstcmd->au8Param[0]] = pstcmd->au8Param[1];
        }
        break;
    }
    case CUS_PLIMIT_GET_ENV_TEMP:
    {
        HI_U8 buf[2];

        buf[0] = pstcmd->au8Param[0];
        buf[1] = pstdata->TempSafe[buf[0]];

        Plimit_Cmd_WriteI2CSlaveBuf(buf, sizeof(buf));
        break;
    }
    case CUS_PLIMIT_RESET_DATA:
    {
        HI_U16 i;
        for (i = 0; i < pstparam->ChAmount; i++)
        {
            pstdata->Duty[i] = 0x00;
            pstdata->Limit[i] = pstparam->LocalDutyMax;
            pstdata->TempDelta[i] = 0x00;
        }
        break;
    }
    case CUS_PLIMIT_BUTT:
    {
        break;
    }
    default:
    {
        PLIMIT_LOG("App_Plimit_Cmd() Error : Unsupported I2C Command.");
        return PLIMIT_FAIL;
    }
    }

    return PLIMIT_SUCCESS;
}
