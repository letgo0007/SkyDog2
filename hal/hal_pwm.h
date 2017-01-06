/**@file    hal_pwm.h
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 2017Äê1ÔÂ5ÈÕ | GKX100052 | Initial Version
 */

#ifndef HAL_HAL_PWM_H_
#define HAL_HAL_PWM_H_

#include <msp430.h>
#include "board.h"

extern void PwmOut_init(void);

extern void PwmOut_setOutput(unsigned char ch, unsigned int freq, unsigned int duty);

extern void PwmIn_init(void);

extern unsigned int PwmIn_getFreq(unsigned char ch);

extern unsigned int PwmIn_getDuty(unsigned char ch);

#endif /* HAL_HAL_PWM_H_ */
