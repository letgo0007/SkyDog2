/******************************************************************************
 * @file    app_plimit.h
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

#ifndef APP_APP_PLIMIT_H_
#define APP_APP_PLIMIT_H_

#define PLIMIT_LIB_USE_ON_MSP430            //Use case for TI MSP430 MCU
//#define PLIMIT_LIB_USE_ON_HISILICON       //Use case for HISILICON M5

/***TI MSP430F5xxx Use case Header********************************************/
#ifdef  PLIMIT_LIB_USE_ON_MSP430
//Includes
#include "msp430.h"
#include "hal.h"

//Type define link to TI 16bit MCU Type.
typedef unsigned char HI_U8;
typedef char HI_S8;
typedef unsigned int HI_U16;
typedef int HI_S16;
typedef unsigned long HI_U32;
typedef long HI_S32;

//Log System Interface
#define PLIMIT_LOG         printf

#define PLIMIT_CKECK_NULL_POINTER(ptr)  \
    if (PLIMIT_NULL == ptr) \
    {\
        PLIMIT_LOG("\r\n[ERROR][FUNC]: %s[NULL POINTER]: %s",__FUNCTION__,#ptr);\
        return PLIMIT_FAIL;\
    }

#endif

/***HISILICON M5 Use Case Header**********************************************/
#ifdef PLIMIT_LIB_USE_ON_HISILICON

//Includes
#include "hi_common.h"

//Log System Interface
#define PLIMIT_LOG              HI_PRINT

#define PLIMIT_CKECK_NULL_POINTER(ptr)  \
    if (PLIMIT_NULL == ptr) \
    {\
        PLIMIT_LOG("\r\n[ERROR][FUNC]: %s[NULL POINTER]: %s",__FUNCTION__,#ptr);\
        return PLIMIT_FAIL;\
    }

#endif

/***PLIMIT general define*****************************************************/
//Constant Value Defines
#define PLIMIT_NULL                     0L      //NULL pointer
#define PLIMIT_CH_MAX                   128     //Max LED channel support.
#define PLIMIT_DB_PARAM_MAX             64      //Max Database build-in parameter amount.
#define PLIMIT_DUTY_BIT                 12      //Duty bit inside PLIMIT function.
#define PLIMIT_DUTY_MAX                 0x1000  //By default is 12bit.
#define PLIMIT_COEF_BIT                 10      //Coefficient bit inside PLIMIT function.
#define PLIMIT_COEF_BASE                1024    //By default is 10bit.
#define PLIMIT_LIMIT_SKEW               4       //Local Limit function skew rate.

//Control BIT define of [ tPlimit_Param.PlimitEn ]
#define PLIMIT_EN_BIT_GAMMA             0x80
#define PLIMIT_EN_BIT_LD                0x40
#define PLIMIT_EN_BIT_GD                0x20
#define PLIMIT_EN_BIT_TEMP              0x10
#define PLIMIT_EN_BIT_ALL               0x01

//Input/Output format define                    // Example:
#define PLIMIT_FORMAT_8DATA_8PACKET     0x01    // 0xAA, 0xBB,
#define PLIMIT_FORMAT_12DATA_8X2PACKET  0x02    // 0x0A, 0xAA, 0x0B, 0xBB,
#define PLIMIT_FORMAT_12DATA_8X1_5PACKET 0x03   // 0xAA, 0xAB, 0xBB,
#define PLIMIT_FORMAT_12DATA_16PACKET   0x04    // 0x0AAA, 0x0BBB

#define PLIMIT_INPUT_FORMAT             PLIMIT_FORMAT_8DATA_8PACKET
#define PLIMIT_OUTPUT_FORMAT            PLIMIT_FORMAT_12DATA_8X1_5PACKET

//API return value define.
typedef enum PLIMIT_RET
{
    PLIMIT_FAIL = 0x00, PLIMIT_SUCCESS = 0x01
} PLIMIT_RET;

//Value of DC share mode for Global Limit function.
typedef enum ePlimit_DcShareMode
{
    DC_SHARE_NONE = 0x00, DC_SHARE_LR2DIV = 0x01, DC_SHARE_TB2DIV = 0x02,

    DC_SHARE_BUTT = 0xFF,
} ePlimit_DcShareMode;

//Structure of PLIMIT function data.
typedef struct tPlimit_Data
{
    //Duty Table
    HI_U16 Duty[PLIMIT_CH_MAX];
    //Duty Limit Table
    HI_U16 Limit[PLIMIT_CH_MAX];
    //Delta Temperature Table , unit in C * PLIMIT_DUTY_MAX (e.g.  0x5000 = 5 C)
    HI_U32 TempDelta[PLIMIT_CH_MAX];
    //Environment Temperature TAble , unit in C * PLIMIT_DUTY_MAX (e.g.  0x5000 = 5 C)
    HI_U8 TempSafe[PLIMIT_CH_MAX];
} tPlimit_Data;

//Structure of PLIMIT function parameters.
typedef struct tPlimit_Param
{
    /*[BIT7] : Gamma function Enable.
     *[BIT6] : Local Duty limit function Enable.
     *[BIT5] : Global Duty limit function Enable.
     *[BIT4] : Temperature limit function Enable.
     *[BIT0] : Overall Enable.
     */
    HI_U8 PlimitEn;
    //[0x00~0xFFFF] PLIMIT function count , usually unit in SEC.
    HI_U16 PlimitCount;
    //[0x01 ~ 0xFF] : Actual used Duty channel amount.
    HI_U8 ChAmount;
    //[0x01 ~ 0xFF] : Duty channel Column.
    HI_U8 ChCol;
    //[0x01 ~ 0xFF] : Duty channel Row.
    HI_U8 ChRow;
    //[0x00] : All Channel use 1 DC power ; [0x01] : 2 DC power (L/R divide) ; [0x02] : 2 DC power (T/B divide)
    ePlimit_DcShareMode ChDcShareMode;
    //[0x00 ~ 0xFF] : Frame rate of duty data per second.
    HI_U8 FrameRate;
    //[0x01 ~ 0xFF] : Frame amount to run temperature calculation. Generally set to 1s.
    HI_U8 FramePerSample;
    //[0x0000 ~ PLIMIT_DUTY_MAX-1] : Pointer to Input gamma table of 256 points.
    HI_U16 *GammaTable;
    //[0x0000 ~ PLIMIT_DUTY_MAX-1] : Maximum average duty of all channels.
    HI_U32 GlobalDutyMax; //0x800
    //[0x00 ~ 0xFF] : Current value , refer to hardware spec.
    HI_U8 GlobalCurrent; //0x800
    //[0x00~0xFF]   : Coefficient of temperature drop.
    HI_U32 CoefD;         //230/256 = 0.9
    //[0x00~0xFFFF] : Coefficient of temperature rise.
    HI_U32 CoefR;         //1024/256 = 4 C / S
    //[0x00~0xFF*0x0100] : Low Temperature value , unit in C * PLIMIT_DUTY_MAX .
    HI_U32 TempLowGap;
    //[0x00~0xFF*0x0100] : High Temperature value , unit in C * PLIMIT_DUTY_MAX .
    HI_U32 TempHighGap;
    //[0x00~PLIMIT_DUTY_MAX] : Absolute Max Duty , usually determined by LED current & Brightness.
    HI_U16 LocalDutyMax;
    //[0x00~PLIMIT_DUTY_MAX] : Maximum Limit Gap between lowest & highest channels.
    HI_U16 LocalDutyLimitGap;
    //[0x00~PLIMIT_DUTY_MAX] : Duty @ maximum temperature allowed for each area , e.g. 0x800 @ 55C.
    HI_U16 *DutyTempSafePtr;
    //[0x00~PLIMIT_DUTY_MAX] : Duty limit go up step per second.
    HI_U16 StepLimitUp;
    //[0x00~PLIMIT_DUTY_MAX] : Duty limit go down step per second.
    HI_U16 StepLimitDown;
} tPlimit_Param;

//PLIMIT database param.
typedef struct tPlimit_Db
{
    HI_U8 current_index;
    HI_U8 total_index;
    tPlimit_Param *pstParamList[PLIMIT_DB_PARAM_MAX];
} tPlimit_Db;

//Global working parameters
#if 1
static tPlimit_Param g_stPlimitParam =
{ 0 };
static tPlimit_Data g_stPlimitData =
{ 0 };
#endif

/***External Functions********************************************************/

/*!@fn    App_Plimit_setEnvTemp
 * @brief Set the environment temperature of each LED local area.
 *
 * @param pstdata       is the pointer to tPlimit_Data.
 * @param pstparam      is the pointer to tPlimit_Param.
 * @return PLIMIT_SUCCESS or PLIMIT_FAIL of the progress.
 */
extern PLIMIT_RET App_Plimit_setSafeTemp(tPlimit_Data *pstdata, tPlimit_Param *pstparam);

/*!@fn    App_Plimit_setGammaTable
 * @brief Set input gamma table with 9 point (0x00 0x20 0x40 0x60 0x80 0xA0 0xC0 0xE0 0x100 )
 *        Convert 9 point gamma to 256 point gamma.
 *
 * @param pstdata       is the pointer to tPlimit_Data.
 * @param pstparam      is the pointer to tPlimit_Param.
 * @return PLIMIT_SUCCESS or PLIMIT_FAIL of the progress.
 */
extern PLIMIT_RET App_Plimit_setGammaTable(tPlimit_Data *pstdata, tPlimit_Param *pstparam);

/*!@fn    App_Plimit
 * @brief The main structure of Power Limit function.
 *        Generally this function should be called every frame.
 * @note  The duty data is 12bit length by default inside this function.
 *        If you use different data length of input or output , modify Plimit_getInputDuty & Plimit_SetOutputDuty function.
 *
 * @param pu16input     is the pointer of input data.
 * @param pu16output    is the pointer to buffer to store output data.
 * @param pstdata       is the pointer to tPlimit_Data.
 * @param pstparam      is the pointer to tPlimit_Param.
 * @return PLIMIT_SUCCESS or PLIMIT_FAIL of the progress.
 */
extern PLIMIT_RET App_Plimit(void *pu16input, void *pu16output, tPlimit_Data *pstdata, tPlimit_Param *pstparam);

/*!@fn    App_PlimitPrint
 * @brief Print out PLIMIT data & parameter struct with certain format.
 *
 * @param pstdata       is the pointer to tPlimit_Data.
 * @param pstparam      is the pointer to tPlimit_Param.
 * @return PLIMIT_SUCCESS or PLIMIT_FAIL of the progress.
 */
extern PLIMIT_RET App_PlimitPrint(tPlimit_Data *pstdata, tPlimit_Param *pstparam);

/*!@fn    App_Plimit_resetDataBuf
 * @brief Reset Data buffer to initial value.
 *        Duty & Temperature buffer are cleared to 0.
 *        Limit is set to maximum allowed.
 *        Environment Temperature has no operation , it is safer to use App_Plimit_setEnvTemp to set.
 *
 * @param pstdata       is the pointer to tPlimit_Data.
 * @param pstparam      is the pointer to tPlimit_Param.
 * @return PLIMIT_SUCCESS or PLIMIT_FAIL of the progress.
 */
extern PLIMIT_RET App_Plimit_resetDataBuf(tPlimit_Data *pstdata, tPlimit_Param *pstparam);

#endif /* API_plimit_H_ */

