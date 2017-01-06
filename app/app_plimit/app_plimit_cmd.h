/******************************************************************************
 * @file    app_plimit_cmd.h
 *
 * I2C command interface of PLIMIT function.
 *
 * @pre     Relay on Relay on [app_plimit.h] & [app_plimit_db.h]
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20161212 | Yang.Zf | Initial Version
 *****************************************************************************/

#ifndef APP_APP_PLIMIT_CMD_H_
#define APP_APP_PLIMIT_CMD_H_

#include "app_plimit.h"
#include "app_plimit_db.h"

//I2C Sub Command ID index for PLIMIT function.
typedef enum ePlimitSubCmd
{
    CUS_PLIMIT_SET_ENABLE = 0x00, CUS_PLIMIT_GET_ENABLE = 0x01,

    CUS_PLIMIT_SET_PARAM_INDEX = 0x02, CUS_PLIMIT_GET_PARAM_INDEX = 0x03,

    CUS_PLIMIT_SET_SAMPLE_RATE = 0x04, CUS_PLIMIT_GET_SAMPLE_RATE = 0x05,

    CUS_PLIMIT_SET_BL_GAMMA_POINT = 0x10, CUS_PLIMIT_GET_BL_GAMMA_POINT = 0x11,

    CUS_PLIMIT_SET_DUTY_PARAM = 0x12, CUS_PLIMIT_GET_DUTY_PARAM = 0x13,

    CUS_PLIMIT_SET_TEMP_COEF = 0x14, CUS_PLIMIT_GET_TEMP_COEF = 0x15,

    CUS_PLIMIT_SET_TEMP_THLD = 0x16, CUS_PLIMIT_GET_TEMP_THLD = 0x17,

    CUS_PLIMIT_SET_TEMP_STEP = 0x18, CUS_PLIMIT_GET_TEMP_STEP = 0x19,

    CUS_PLIMIT_SET_TEMP_SAFE_DUTY = 0x1A, CUS_PLIMIT_GET_TEMP_SAFE_DUTY = 0x1B,

    CUS_PLIMIT_SET_ENV_TEMP = 0x1C, CUS_PLIMIT_GET_ENV_TEMP = 0x1D,

    CUS_PLIMIT_RESET_DATA = 0x1E,

    CUS_PLIMIT_BUTT = 0xFF

} ePlimitSubCmd;

/*!@fn      App_Plimit_Cmd
 * @brief   Handle I2C command interface for PLIMIT function.
 *
 * @param   pu8cmd      is the pointer to I2C command structure.
 * @param   pstdata     is the pointer to Plimit data structure.
 * @param   pstparam    is the pointer to Plimit parameter structure.
 * @param   pstdb       is the pointer to Plimit database structure.
 * @return  PLIMIT_SUCCESS or PLIMIT_FAIL of the progress.
 */
extern PLIMIT_RET App_Plimit_Cmd(HI_U8 *pu8cmd, tPlimit_Data *pstdata, tPlimit_Param *pstparam, tPlimit_Db *pstdb);

#endif /* APP_APP_PLIMIT_CMD_H_ */
