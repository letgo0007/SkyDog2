/******************************************************************************
 * @file    app_plimit.c
 *
 * LED Driver power limit function.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20160908 | Yang.Zf | Initial Version.
 * 20161210 | Yang.Zf | Move buffers into tPlimit_Data structure.
 *                      This help reduce RAM size used when switch work mode.
 *                      Add 8bit input/output mode.
 * 20161212 | Yang.Zf | Add ChDcShareMode for 2 DC power share model.
 *                      Add LocalDutyLimitGap to set the maximum difference between channels.
 *
 *****************************************************************************/

#include "app_plimit.h"
#include "app_plimit_db.h"

/***Internal Functions********************************************************/

inline HI_U16 Plimit_Max(HI_U16 a, HI_U16 b)
{
    return a > b ? a : b;
}
inline HI_U16 Plimit_Min(HI_U16 a, HI_U16 b)
{
    return a < b ? a : b;
}

PLIMIT_RET Plimit_getInputDuty(void *pvduty, tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pvduty);
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    HI_U16 i;

    //1 Handle data convert.
#if PLIMIT_INPUT_FORMAT == PLIMIT_FORMAT_8DATA_8PACKET
    HI_U8* pu8duty = &*pvduty;

    for (i = 0; i < pstparam->ChAmount; i++)
    {
        //0xAB -> 0x0AB0
        pstdata->Duty[i] = pu8duty[i] << 4;
    }

#endif
#if PLIMIT_INPUT_FORMAT == PLIMIT_FORMAT_12DATA_8X2PACKET
    HI_U8* pu8duty = &*pvduty;
    HI_U16 j = 0;

    for (i = 0; i < pstparam->ChAmount; i++)
    {
        //0x0A , 0xBC -> 0x0ABC
        pstdata->Duty[i] = ((HI_U16) pu8duty[j++] << 8) + pu8duty[j++];
    }

#endif
#if PLIMIT_INPUT_FORMAT == PLIMIT_FORMAT_12DATA_8X1_5PACKET
    HI_U8* pu8duty = &*pvduty;
    HI_U16 j = 0;

    for (i = 0; i < pstparam->ChAmount;)
    {
        //0xAB , 0xCD , 0xEF -> 0x0ABC , 0x0DEF
        pstdata->Duty[i] = ((HI_U16) pu8duty[j] << 4) + (pu8duty[j + 1] >> 4);
        pstdata->Duty[i + 1] = ((HI_U16) (pu8duty[j + 1] & 0x0F) << 8) + pu8duty[j + 2];
        i = i + 2;
        j = j + 3;
    }

#endif
#if PLIMIT_INPUT_FORMAT == PLIMIT_FORMAT_12DATA_16PACKET
    HI_U16* pu16duty = &*pvduty;

    for (i = 0; i < pstparam->ChAmount; i++)
    {
        pstdata->Duty[i] = pu16duty[i];
    }

#endif

    //2 Handle Gamma.
    if (pstparam->PlimitEn & PLIMIT_EN_BIT_GAMMA)
    {
        PLIMIT_CKECK_NULL_POINTER(pstparam->GammaTable);

        for (i = 0; i < pstparam->ChAmount; i++)
        {
            pstdata->Duty[i] = pstparam->GammaTable[pstdata->Duty[i] >> 4];
        }
    }

    return PLIMIT_SUCCESS;
}

PLIMIT_RET Plimit_setOutputDuty(void *pvoutput, tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pvoutput);
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    HI_U16 i = 0;

#if PLIMIT_OUTPUT_FORMAT == PLIMIT_FORMAT_8DATA_8PACKET
    HI_U8 * pu8output = &*pvoutput;
    for (i = 0; i < pstparam->ChAmount; i++)
    {
        // 0x0ABC -> 0xAB
        pu8output[i] = (pstdata->Duty[i]) >> 4;
    }
#endif
#if PLIMIT_OUTPUT_FORMAT == PLIMIT_FORMAT_12DATA_8X2PACKET
    HI_U8 * pu8output = &*pvoutput;
    HI_U16 j = 0;
    for (i = 0; i < pstparam->ChAmount; i++)
    {
        // 0x0ABC -> 0x0A , 0x0B
        pu8output[j ++] = (pstdata->Duty[i] & 0x0F00 ) >> 8;
        pu8output[j ++] = (pstdata->Duty[i]) & 0xFF;
    }
#endif
#if PLIMIT_OUTPUT_FORMAT == PLIMIT_FORMAT_12DATA_8X1_5PACKET
    HI_U8 * pu8output = &*pvoutput;
    HI_U16 j = 0;
    for (i = 0; i < pstparam->ChAmount; i = i + 2)
    {
        // 0x0ABC 0x0DEF -> 0xAB, 0xCD, 0xEF
        pu8output[j++] = pstdata->Duty[i] >> 4;
        pu8output[j++] = ((pstdata->Duty[i] & 0x0F) << 4) + ((pstdata->Duty[i + 1] & 0x0F00) >> 8);
        pu8output[j++] = (pstdata->Duty[i + 1]) & 0xFF;
    }
#endif
#if PLIMIT_OUTPUT_FORMAT == PLIMIT_FORMAT_12DATA_16PACKET
    HI_U16 * pu16output = &*pvoutput;

    for (i = 0; i < pstparam->ChAmount; i++)
    {
        // 0x0ABC
        pu16output[i] = pstdata->Duty[i];
    }
#endif

    return PLIMIT_SUCCESS;
}

PLIMIT_RET Plimit_doLocalLimit(tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    HI_U16 i;

    for (i = 0; i < pstparam->ChAmount; i++)
    {
        /* Limit Curve :
         * |Output        _
         * |        _  +        -> skew = 1/4
         * |     +
         * |   +
         * | +                  -> skew = 1
         * |______________Input
         *
         */
        pstdata->Duty[i] = Plimit_Min(pstdata->Duty[i],
                pstdata->Duty[i] / PLIMIT_LIMIT_SKEW + pstdata->Limit[i] - PLIMIT_DUTY_MAX / PLIMIT_LIMIT_SKEW);
    }

    return PLIMIT_SUCCESS;
}

PLIMIT_RET Plimit_doGlobalLimit(tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    HI_U16 i, j;
    HI_U32 sum = 0;
    HI_U32 sum_t = 0;   //Sum buffer used only when DC share mode.
    HI_U32 avg = 0;
    HI_U32 avg_t = 0;   //Average buffer used only when DC share mode.
    HI_U32 gain = 0;

    //1 Get average duty according to DC share mode
    switch (pstparam->ChDcShareMode)
    {
    case DC_SHARE_NONE:
    {
        //Get Average for all channels.
        for (i = 0; i < pstparam->ChAmount; i++)
        {
            sum += pstdata->Duty[i];
        }
        avg = sum / pstparam->ChAmount;

        break;
    }

    case DC_SHARE_LR2DIV:
    {
        //Get Average for left side
        for (i = 0; i < pstparam->ChRow; i++)
        {
            for (j = 0; j < pstparam->ChCol / 2; j++)
            {
                sum_t += pstdata->Duty[i * pstparam->ChCol + j];
            }
        }
        avg_t = sum_t / (pstparam->ChAmount / 2);

        //Get Average for right side
        for (i = 0; i < pstparam->ChRow; i++)
        {
            for (j = pstparam->ChCol / 2; j < pstparam->ChCol; j++)
            {
                sum += pstdata->Duty[i * pstparam->ChCol + j];
            }
        }
        avg = sum / (pstparam->ChAmount / 2);

        //Get overall average = the lower of Left/Right .
        avg = Plimit_Min(avg, avg_t);

        break;
    }
    case DC_SHARE_TB2DIV:
    {
        //Get Average for top side
        for (i = 0; i < pstparam->ChRow / 2; i++)
        {
            for (j = 0; j < pstparam->ChCol; j++)
            {
                sum_t += pstdata->Duty[i * pstparam->ChCol + j];
            }
        }
        avg_t = sum_t / ((pstparam->ChAmount) / 2);

        //Get Average for bottom side
        for (i = pstparam->ChRow / 2; i < pstparam->ChRow; i++)
        {
            for (j = 0; j < pstparam->ChCol; j++)
            {
                sum += pstdata->Duty[i * pstparam->ChCol + j];
            }
        }
        avg = sum / ((pstparam->ChAmount) / 2);

        //Get overall average = the lower of Top/Bottom .
        avg = Plimit_Min(avg, avg_t);
        break;
    }
    default:
        //Not supported DC share mode, return fail.
        PLIMIT_LOG("\r\n Plimit_doGlobalLimit() Error : Not supported DC share mode. \r\n");
        return PLIMIT_FAIL;
    }

    //2 Calculate Gain with 10bit resolution.
    gain = (pstparam->GlobalDutyMax << PLIMIT_COEF_BIT) / avg;

    //3 Multiple every local duty by gain if needed.
    if (gain <= PLIMIT_COEF_BASE)
    {
        for (i = 0; i < pstparam->ChAmount; i++)
        {
            pstdata->Duty[i] = (gain * pstdata->Duty[i]) >> PLIMIT_COEF_BIT;
        }
    }

    return PLIMIT_SUCCESS;
}

PLIMIT_RET Plimit_doTempEstimate(tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    HI_U16 i;
    for (i = 0; i < pstparam->ChAmount; i++)
    {
        /* Core Algorithm :
         * ----------------------------------------------
         * Temp[n] = Temp[n-1] * CoefD + Duty[n] * CoefR
         * ----------------------------------------------
         * Temp[n]  : Current Temperature , unit in C * PLIMIT_DUTY_MAX(12bit).
         * Temp[n-1]: Last time Temperature, unit in C * PLIMIT_DUTY_MAX(12bit).
         *            e.g.  0x1000 = 1C , 0x32000 = 50C.
         * CoefD    : Coefficient of temperature drop , valid values are 1~1023/1024.
         *            e.g.  1000 = 0.977f
         * CoefR    : Coefficient of temperature rise , valid values are 1~0xFFFF/1024.
         *            e.g.  2000 = 1.953f C/S.
         * Duty[n]  : Current Duty, valid values are 0 ~ PLIMIT_DUTY_MAX(12bit).
         */
        pstdata->TempDelta[i] = ((pstdata->TempDelta[i] * pstparam->CoefD + PLIMIT_COEF_BASE / 2) >> PLIMIT_COEF_BIT)
                + ((pstdata->Duty[i] * pstparam->CoefR + PLIMIT_COEF_BASE / 2) >> PLIMIT_COEF_BIT);
    }

    return PLIMIT_SUCCESS;
}

PLIMIT_RET Plimit_doLimitDecision(tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);
    PLIMIT_CKECK_NULL_POINTER(pstparam->DutyTempSafePtr);

    HI_U16 i;
    for (i = 0; i < pstparam->ChAmount; i++)
    {
        /* Limit control decision logic :
         * Section          | Condition         | Action
         * ---------------------------------------------------------------------------
         * Low Temp         | Temp < 40         | Limit ++ with step, MAX = DutyMax.
         * Warning Temp     | 40 < Temp < 50    | Limit -- with step, MIN = DutyHighTemp.
         * High Temp        | 50 < Temp         | Limit fix to DutyHighTemp.
         * ---------------------------------------------------------------------------
         */

        //Calculate absolute temperature.
        HI_U16 temp_ab = (pstdata->TempDelta[i] >> PLIMIT_DUTY_BIT);

        //Low temperature , limit go up by step.
        if (temp_ab < pstdata->TempSafe[i] - pstparam->TempLowGap)
        {
            pstdata->Limit[i] = Plimit_Min(pstdata->Limit[i] + pstparam->StepLimitUp, pstparam->LocalDutyMax);
        }
        //Warning temperature , limit go down by step.
        else if (temp_ab < pstdata->TempSafe[i] - pstparam->TempHighGap)
        {
            pstdata->Limit[i] = Plimit_Max((pstdata->Limit[i] - pstparam->StepLimitDown), pstparam->DutyTempSafePtr[i]);
        }
        //High temperature , limit fix to DutyHighTemp.
        else
        {
            pstdata->Limit[i] = pstparam->DutyTempSafePtr[i];
        }
    }

    //Find minimum limit
    HI_U16 limit_min = pstparam->LocalDutyMax;

    for (i = 0; i < pstparam->ChAmount; i++)
    {
        limit_min = Plimit_Min(limit_min, pstdata->Limit[i]);
    }

    //Set the maximum gap between each local limit.
    HI_U16 limit_max = limit_min + pstparam->LocalDutyLimitGap;

    for (i = 0; i < pstparam->ChAmount; i++)
    {
        pstdata->Limit[i] = Plimit_Min(limit_max, pstdata->Limit[i]);
    }

    return PLIMIT_SUCCESS;
}

/***External Functions********************************************************/

PLIMIT_RET App_PlimitPrint(tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    HI_U8 temp[PLIMIT_CH_MAX];
    HI_U8 i;

    PLIMIT_LOG("\r\nPLIMIT Runtime : %d", pstparam->PlimitCount);
    PLIMIT_LOG("\r\n\t[CH]\t[DUTY]\t[LIMIT]\t[TEMP]\r\n----------------------------\r\n");
    for (i = 0; i < pstparam->ChAmount; i++)
    {
        temp[i] = pstdata->TempDelta[i] >> PLIMIT_DUTY_BIT;
        PLIMIT_LOG("\t[%d]\t[%x]\t[%x]\t[%d]\r\n", i, pstdata->Duty[i], pstdata->Limit[i], temp[i]);
    }

    return PLIMIT_SUCCESS;
}

PLIMIT_RET App_Plimit_setSafeTemp(tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    HI_U16 i;
    for (i = 0; i < pstparam->ChAmount; i++)
    {
        /* Get Safe temperature according to DutyTempSafePtr, CoefR & CoefD
         * ----------------------------------------------------------------
         *  TempSafe = DutyTempSafe * CoefR / (1-CoefD)
         * ----------------------------------------------------------------
         * Generally the value will locate around 55C.
         * Also please note the bit number of each coefficient.
         */
        pstdata->TempSafe[i] = (pstparam->DutyTempSafePtr[i] * pstparam->CoefR / (PLIMIT_COEF_BASE - pstparam->CoefD))
                >> PLIMIT_DUTY_BIT;
    }
    return PLIMIT_SUCCESS;
}

PLIMIT_RET App_Plimit_setGammaTable(tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);
    PLIMIT_CKECK_NULL_POINTER(pstparam->GammaTable);

    /*Convert 9 point Gamma (Include 0x00 & 0xFF ) to 256 point Gamma .
     *Gamma Point are located at 0x00 0x20 0x40 0x60 0x80 0xA0 0xC0 0xE0 0x100
     */
    HI_U16 i;
    HI_U16 j;
    HI_U16 step;

    for (i = 0; i < 8; i++)
    {
        step = (pstparam->GammaTable[(i + 1) * 0x200] - pstparam->GammaTable[i * 0x200]) / 0x20;

        for (j = 0; j < 0x20; j++)
        {
            pstparam->GammaTable[0x200 * i + j] = pstparam->GammaTable[0x200 * i] + step * j;
        }
    }
    return PLIMIT_SUCCESS;
}

PLIMIT_RET App_Plimit_resetDataBuf(tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    HI_U16 i;
    for (i = 0; i < pstparam->ChAmount; i++)
    {
        pstdata->Duty[i] = 0x00;
        pstdata->Limit[i] = pstparam->LocalDutyMax;
        pstdata->TempDelta[i] = 0x00;
    }
    return PLIMIT_SUCCESS;
}

PLIMIT_RET App_Plimit_init(HI_U16 param_index, tPlimit_Data *pstdata, tPlimit_Param *pstparam, tPlimit_Db *pstdb)
{
    App_Plimit_Db_setParamIndex(param_index, pstparam, pstdb);
    App_Plimit_resetDataBuf(pstdata, pstparam);
    App_Plimit_setSafeTemp(pstdata, pstparam);

    return PLIMIT_SUCCESS;
}

PLIMIT_RET App_Plimit(void *pvinput, void *pvoutput, tPlimit_Data *pstdata, tPlimit_Param *pstparam)
{
    PLIMIT_CKECK_NULL_POINTER(pvinput);
    PLIMIT_CKECK_NULL_POINTER(pvoutput);
    PLIMIT_CKECK_NULL_POINTER(pstdata);
    PLIMIT_CKECK_NULL_POINTER(pstparam);

    static HI_U16 framecount = 0;

    if (pstparam->PlimitEn & PLIMIT_EN_BIT_ALL)
    {
        //Do Power limit function every frame.
        Plimit_getInputDuty(pvinput, pstdata, pstparam);
        if (pstparam->PlimitEn & PLIMIT_EN_BIT_LD)
        {
            Plimit_doLocalLimit(pstdata, pstparam);
        }
        if (pstparam->PlimitEn & PLIMIT_EN_BIT_GD)
        {
            Plimit_doGlobalLimit(pstdata, pstparam);
        }
        Plimit_setOutputDuty(pvoutput, pstdata, pstparam);

        //Do Temperature control function every 60 frames(1s).
        if (pstparam->PlimitEn & PLIMIT_EN_BIT_TEMP)
        {
            framecount++;
            if (framecount >= pstparam->FramePerSample)
            {
                Plimit_doTempEstimate(pstdata, pstparam);
                Plimit_doLimitDecision(pstdata, pstparam);
                pstparam->PlimitCount++;
                framecount = 0;
            }
        }
    }
    else
    {
        //Bypass input to output.
        memcpy(pvoutput, pvinput, 128);
    }

    return PLIMIT_SUCCESS;
}

PLIMIT_RET App_Plimit_Db_setParamIndex(HI_U8 u8index, tPlimit_Param *pstparam, tPlimit_Db *pstdb)
{
    PLIMIT_CKECK_NULL_POINTER(pstparam);
    PLIMIT_CKECK_NULL_POINTER(pstdb);

    //Store current index.
    pstdb->current_index = u8index;

    //Copy structure by index.
    memcpy(pstparam, pstdb->pstParamList[pstdb->current_index], sizeof(tPlimit_Param));
    PLIMIT_LOG("Load PLIMIT build-in param index:[ %d ]\r\n", pstdb->current_index);

    return PLIMIT_SUCCESS;
}

PLIMIT_RET App_Plimit_Db_getParamIndex(HI_U8 *current_index, tPlimit_Db *pstdb)
{
    PLIMIT_CKECK_NULL_POINTER(current_index);
    PLIMIT_CKECK_NULL_POINTER(pstdb);

    *current_index = pstdb->current_index;
    return PLIMIT_SUCCESS;
}

PLIMIT_RET APP_PLIMIT_setSafeTemp(void)
{
    return App_Plimit_setSafeTemp(&g_stPlimitData, &g_stPlimitParam);
}
PLIMIT_RET APP_PLIMIT_setGammaTable(void)
{
    return App_Plimit_setGammaTable(&g_stPlimitData, &g_stPlimitParam);
}

PLIMIT_RET APP_PLIMIT(void *pu16input, void *pu16output)
{
    return App_Plimit(pu16input, pu16output, &g_stPlimitData, &g_stPlimitParam);
}

PLIMIT_RET APP_PLIMIT_Print(void)
{
    return App_PlimitPrint(&g_stPlimitData, &g_stPlimitParam);
}

PLIMIT_RET APP_PLIMIT_resetDataBuf(void)
{
    return App_Plimit_resetDataBuf(&g_stPlimitData, &g_stPlimitParam);
}
PLIMIT_RET APP_PLIMIT_DB_setParamIndex(HI_U8 u8index)
{
    return App_Plimit_Db_setParamIndex(u8index, &g_stPlimitParam, &g_stPlimitDb);
}

PLIMIT_RET APP_PLIMIT_DB_getParamIndex(HI_U8 *current_index)
{
    return App_Plimit_Db_getParamIndex(current_index, &g_stPlimitDb);
}

PLIMIT_RET APP_PLIMIT_init(HI_U16 param_index)
{
    return App_Plimit_init(param_index, &g_stPlimitData, &g_stPlimitParam, &g_stPlimitDb);
}

