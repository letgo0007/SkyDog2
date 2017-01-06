/******************************************************************************
 * @file    app_plimit_cmd.c
 *
 * PLIMIT function build-in parameter database.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20161212 | Yang.Zf | Initial Version
 *****************************************************************************/
#include "app_plimit_db.h"

/***External Functions********************************************************/
PLIMIT_RET App_Plimit_Db_setParamIndex(HI_U8 u8index, tPlimit_Param *pstparam, tPlimit_Db *pstdb)
{
    PLIMIT_CKECK_NULL_POINTER(pstparam);
    PLIMIT_CKECK_NULL_POINTER(pstdb);

    //Store current index.
    pstdb->current_index = u8index;

    //Copy structure by index.
    memcpy(pstparam, pstdb->pstParamList[pstdb->current_index], sizeof(tPlimit_Param));
    PLIMIT_LOG("Load PLIMIT build-in param index:[ %d ]\r\n",pstdb->current_index);

    return PLIMIT_SUCCESS;
}

PLIMIT_RET App_Plimit_Db_getParamIndex(HI_U8 *current_index, tPlimit_Db *pstdb)
{
    PLIMIT_CKECK_NULL_POINTER(current_index);
    PLIMIT_CKECK_NULL_POINTER(pstdb);

    *current_index = pstdb->current_index;
    return PLIMIT_SUCCESS;
}
