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

extern uint8_t App_Cmd_handleI2c(void);

extern uint8_t App_Cmd_handleUart(void);

#endif /* APP_APP_CMD_H_ */
