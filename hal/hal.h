/**@file    hal.h
 *
 * MSP430F5xxx overall MCU HAL level code.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#ifndef HAL_HAL_H_
#define HAL_HAL_H_

#include "hal_adc.h"
#include "hal_clock.h"
#include "hal_uart.h"
#include "hal_spi.h"
#include "hal_pwm.h"
#include "hal_i2c.h"

/*!@brief   Initialize MCU .
 *
 * @param   bon  : [1] MCU turn on.
 *                 [0] MCU turn off.
 */
extern void Mcu_init(unsigned char bon);

/*!@brief   Reset MCU with a software POR (Power On Reset)
 */
extern void Mcu_reset(void);

#endif /* HAL_HAL_H_ */
