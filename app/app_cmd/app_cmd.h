/**@file    app_cmd.h
 *
 * UART & I2C control interface handler.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20161227 | Yang.Zf | Initial Version.
 */

#ifndef APP_APP_CMD_H_
#define APP_APP_CMD_H_

#include "stdint.h"

/*!@brief   Get Command from I2C Slave & process.
 */
extern void App_Cmd_I2c(void);

/*!@brief   Get Command from UART & process.
 */
extern void App_Cmd_Uart(void);

#endif /* APP_APP_CMD_H_ */
