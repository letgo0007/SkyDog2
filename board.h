/**@file    board.h
 *
 * Board hardware pin define & initialization functions.
 *
 * Copyright (c) 2016 SHARP CORPORATION
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <msp430.h>

/******************************************************************************
 * General values
 *****************************************************************************/
#define BOARD_NAME          "SKYDOG"
#define BOARD_VER           0x1701040A

#define ON                  1
#define OFF                 0
#define STATUS_SUCCESS      1
#define STATUS_FAIL         0
#define HIGH                1
#define LOW                 0

/******************************************************************************
 * Clock & XTAL hardware define.
 *****************************************************************************/
#define XT1_F               0                   //ACLK second source
#define XT2_F               0                   //MCLK/SMCLK second source
#define SET_XT1_FUNC_PIN    P5SEL |= BIT2+BIT3  //Set XT1 function pin
#define SET_XT2_FUNC_PIN    P5SEL |= BIT4+BIT5  //Set XT2 function pin

#define REFO_F              (32768)
#define MCLK_F_4M           (3997696)
#define MCLK_F_4M2          (4194304)
#define MCLK_F_8M           (7995392)
#define MCLK_F_8M4          (8388608)
#define MCLK_F_16M          (15990784)
#define MCLK_F_16M8         (16777216)
#define MCLK_F              MCLK_F_16M8         //MCLK for CPU

#define SMCLK_DIV_BIT       2
#define SMCLK_F             (MCLK_F>>SMCLK_DIV_BIT) //SMCLK for UART/I2C/SPI

#define ACLK_F_32K          REFO_F
#define ACLK_F              ACLK_F_32K          //ACLK for RTC/TIMER

/******************************************************************************
 * USCI define.
 *****************************************************************************/
#define SPI_SLAVE_BASE      USCI_A0_BASE
#define SPI_MASTER_BASE     USCI_B1_BASE
#define UART_BASE           USCI_A1_BASE
#define I2C_MASTER_BASE     0
#define I2C_SLAVE_BASE      USCI_B0_BASE

#define SPI_MASTER_CLK      1000000
#define UART_BAUDRATE       115200
#define I2C_MASTER_CLK      100000
#define I2C_SLAVE_ADDRESS   0x48

/******************************************************************************
 * PWM define.
 *****************************************************************************/
#define PWM_IN_TIMER_BASE   TIMER_A1_BASE
#define PWM_OUT_TIMER_BASE  TIMER_A2_BASE

/******************************************************************************
 *@fn       Board_init
 *@brief    Initialize board to ON or OFF status.
 *@param    bon is the operation select.
 *          \b ON  Board turn on.
 *          \b OFF Board turn off.
 *****************************************************************************/
void Board_init(unsigned char bon);

#endif /* BOARD_H_ */
