/**@file    app_player.h
 *
 * Local Dimming LED module display player function .
 * 1 Get Duty from SPI.
 * 2 Set Duty to BL.
 *
 * With 3 build-in SPI Raw data receiving / transmitting function.
 * Custom device need external BSP support ( e.g. IW7027 ).
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170106 | Yang.Zf | Initial Version
 */

#ifndef APP_APP_PLAYER_H_
#define APP_APP_PLAYER_H_

#include "stdint.h"

typedef enum PLAYER_RET
{
    PLAYER_FAIL = 0, PLAYER_SUCCESS = 1,
} PLAYER_RET;

typedef enum PLAYER_INPUT_MODEL
{
    IN_D8_P8 = 0x00,                //RAW DATA 0xAA, 0xBB               -> 0x0AAA, 0x0BBB
    IN_D12_P8X1_5 = 0x01,           //RAW DATA 0xAA, 0xAB, 0xBB         -> 0x0AAA, 0x0BBB
    IN_D12_P8X2 = 0x02,             //RAW DATA 0x0A, 0xAA, 0x0B, 0xBB   -> 0x0AAA, 0x0BBB

    IN_BOTTOM = 0xFF
} PLAYER_INPUT_MODEL;

typedef enum PLAYER_OUTPUT_MODEL
{
    OUT_D8_P8 = 0x00,               //RAW DATA 0x0AAA, 0x0BBB -> 0xAA, 0xBB
    OUT_D12_P8X1_5 = 0x01,          //RAW DATA 0x0AAA, 0x0BBB -> 0xAA, 0xAB, 0xBB
    OUT_D12_P8X2 = 0x02,            //RAW DATA 0x0AAA, 0x0BBB -> 0x0A, 0xAA, 0x0B, 0xBB

    OUT_IW7027_GOA_16X1 = 0x10,     //BL MODEL = 60/70SU870A , SPI connect to IW7027
    OUT_IW7027_SU860A_6X10 = 0x11,  //BL MODEL = 60SU860A , SPI connect to IW7027
    OUT_IW7027_SU860A_6X13 = 0x12,  //BL MODEL = 70SU860A , SPI connect to IW7027

    OUT_CPLD_SU860A_6X10 = 0x20,    //BL MODEL = 60SU860A , SPI connect to CPLD
    OUT_CPLD_SU860A_6X13 = 0x21,    //BL MODEL = 70SU860A , SPI connect to CPLD

    OUT_BOTTOM = 0xFF
} PLAYER_OUTPUT_MODEL;

typedef enum PLAYER_TEST_PATTERN
{
    PTP_MUTE = 0x00,                //0x0000
    PTP_MAX = 0x01,                 //0x0FFF
    PTP_50 = 0x02,                  //0x0800
    PTP_RUN_HORSE= 0x03,            //Run horse

    PTP_BOTTOM = 0xFF
} PLAYER_TEST_PATTERN;

typedef struct PLAYER_PARAM
{
    PLAYER_INPUT_MODEL pin_model;
    PLAYER_OUTPUT_MODEL pout_model;
}PLAYER_PARAM;

extern uint16_t App_Player_getDuty(uint16_t *pu16duty, PLAYER_INPUT_MODEL emodel);

extern PLAYER_RET App_Player_setDuty(uint16_t *pu16duty, uint16_t duty_size, PLAYER_OUTPUT_MODEL emodel);

extern PLAYER_RET App_Player_setTestPattern(PLAYER_TEST_PATTERN eptp, uint16_t duty_size, PLAYER_OUTPUT_MODEL emodel);

extern PLAYER_RET App_Player_printDuty(uint16_t *pu16duty , uint16_t col , uint16_t row );

extern PLAYER_RET App_Player_setWorkParam(PLAYER_PARAM *param);

extern PLAYER_RET App_Player(PLAYER_PARAM *param);

#endif /* APP_APP_PLAYER_H_ */
