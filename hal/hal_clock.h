/**@file    hal_clock.h
 *
 * MSP430F5xx clock system HAL.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#ifndef HAL_HAL_CLOCK_H_
#define HAL_HAL_CLOCK_H_

#include "board.h"

/****************************************************************************
 * Pre-defined Clock working parameters.
 * If not defined in < board.h > , set to a default work mode.
 *****************************************************************************/
#ifndef MCLK_F
#define MCLK_F              1048576
#endif
#ifndef SMCLK_DIV_BIT
#define SMCLK_DIV_BIT       0
#endif
#ifndef ACLK_F
#define ACLK_F              32768
#endif
#ifndef REFO_F
#define REFO_F              32768
#endif
#ifndef XT1_F
#define XT1_F               0
#endif
#ifndef XT2_F
#define XT2_F               0
#endif

/****************************************************************************
 * Operation macros.
 *****************************************************************************/
#define DELAY_MS(ms)        __delay_cycles(ms * MCLK_F / 1000)      /*Delay unit in MS*/
#define DELAY_US(us)        __delay_cycles(us * MCLK_F / 1000000)   /*Delay unit in US*/

#define WATCHDOG_HOLD       WDTCTL = WDTPW | WDTHOLD                /*Watch dog disable */
#define WATCHDOG_FEED       WDTCTL = WDT_ARST_1000                  /*Watch dog feed , ALCK source , 1000ms.*/

#define MCU_RESET           do{ \
        PMMCTL0_H = PMMPW_H; \
        PMMCTL0_L |= PMMSWPOR; \
}while(0)/*MCU reset using soft POR*/

/*Reset MCU by software POR*/

/******************************************************************************
 * @fn      Clock_init
 * @brief   Initialize MSP430 clock system .
 *          The parameters are pre-defined , do not support dynamic change now.
 *****************************************************************************/
extern void Clock_init(void);

#endif /* HAL_HAL_CLOCK_H_ */
