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

/*!@brief   Initialize Timer for PWM output.
 */
extern void PwmOut_init(void);


/*!@brief   Set PWM output working parameters.
 * @note    PWM channel 0 duty is fixed to 50%.
 *
 * @param   ch      : [0~2] PWM output channel.
 * @param   freq    : [Hz] Frequency of PWM.
 *                    [0]  The selected PWM port set to Output Low.
 * @param   duty    : [x/256] Duty of PWM.
 */
extern void PwmOut_setOutput(unsigned char ch, unsigned int freq, unsigned int duty);

/*!@brief   Get PWM out rising edge flag.
 *          When a rising edge has been send, the flag is set to 0x01.
 *          It automatically clears to 0x00 when read.
 *
 * @param   ch      : [0~2] PWM output channel.
 * @return  0x01     : Rising Edge has been sent.
 *          0x00     : NO Rising Edge has been sent since last read.
 */
extern unsigned char PwmOut_getRiseEdgeFlag(unsigned char ch);

/*!@brief   Initialize Timer for PWM input.
 */
extern void PwmIn_init(void);

/*!@brief   Get Frequency of Pwm Input port.
 *
 * @param   ch      : [1~2] PWM input channel.
 * @return  frequency unit in [Hz]
 */
extern unsigned int PwmIn_getFreq(unsigned char ch);

/*!@note not available yet.
 */
extern unsigned int PwmIn_getDuty(unsigned char ch);

/*!@brief   Get PWM input rising edge flag.
 *          When a rising edge has been send, the flag is set to 0x01.
 *          It automatically clears to 0x00 when read.
 *
 * @param   ch      : [0~2] PWM input channel.
 * @return  0x01     : Rising Edge has been captured.
 *          0x00     : NO Rising Edge has been captured since last read.
 */
extern unsigned char PwmIn_getRiseEdgeFlag(unsigned char ch);
#endif /* HAL_HAL_PWM_H_ */
