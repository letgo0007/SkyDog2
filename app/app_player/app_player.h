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
    IN_DISABLE = 0x00,              //Not get any data.
    IN_D8_P8 = 0x01,                //RAW DATA 0xAA, 0xBB               -> 0x0AAA, 0x0BBB
    IN_D12_P8X1_5 = 0x02,           //RAW DATA 0xAA, 0xAB, 0xBB         -> 0x0AAA, 0x0BBB
    IN_D12_P8X2 = 0x03,             //RAW DATA 0x0A, 0xAA, 0x0B, 0xBB   -> 0x0AAA, 0x0BBB

    IN_MFC11_SU860A_6X10 = 0x20,   //SPI_S connect to MFC11, TV model = 60SU860A.
    IN_MFC11_SU860A_6X13 = 0x21,   //SPI_S connect to MFC11, TV model = 70SU860A.
    IN_BOTTOM = 0xFF
} PLAYER_INPUT_MODEL;

typedef enum PLAYER_OUTPUT_MODEL
{
    OUT_DISABLE = 0x00,            //Not send any data.
    OUT_D8_P8 = 0x01,               //RAW DATA 0x0AAA, 0x0BBB -> 0xAA, 0xBB
    OUT_D12_P8X1_5 = 0x02,          //RAW DATA 0x0AAA, 0x0BBB -> 0xAA, 0xAB, 0xBB
    OUT_D12_P8X2 = 0x03,            //RAW DATA 0x0AAA, 0x0BBB -> 0x0A, 0xAA, 0x0B, 0xBB

    OUT_IW7027_GOA_16X1 = 0x10,     //BL MODEL = 60/70SU870A , SPI_M connect to IW7027
    OUT_IW7027_SU860A_6X10 = 0x11,  //BL MODEL = 60SU860A , SPI_M connect to IW7027
    OUT_IW7027_SU860A_6X13 = 0x12,  //BL MODEL = 70SU860A , SPI_M connect to IW7027

    OUT_CPLD_SU860A_6X10 = 0x20,    //BL MODEL = 60SU860A , SPI_M connect to CPLD
    OUT_CPLD_SU860A_6X13 = 0x21,    //BL MODEL = 70SU860A , SPI_M connect to CPLD

    OUT_BOTTOM = 0xFF
} PLAYER_OUTPUT_MODEL;

typedef enum PLAYER_TEST_PATTERN
{
    PTP_DISABLE = 0x00,             //Not run test pattern.
    PTP_MUTE = 0x01,                //0x0000
    PTP_MAX = 0x02,                 //0x0FFF
    PTP_50 = 0x03,                  //0x0800
    PTP_RUN_HORSE = 0x04,           //Run horse

    PTP_BOTTOM = 0xFF
} PLAYER_TEST_PATTERN;

typedef enum PLAYER_SYNC_MODE
{
    PSYNC_OUT_ON_SPI_IN = 0x00,         //Output duty when received valid SPI input data.
    PSYNC_OUT_ON_VSYNC_IN = 0x01,       //Output duty on VSYNC_IN rising edge.
    PSYNC_OUT_ON_VSYNC_OUT = 0x02,      //Output duty on VSYNC_OUT rising edge.
    PSYNC_OUT_ON_INTERNAL_60Hz = 0x03,  //Output duty on Internal 60Hz clock tick.

    PSYNC_BOTTOM = 0xFF
} PLAYER_SYNC_MODE;

typedef struct PLAYER_PARAM
{
    uint16_t pch_amount;
    PLAYER_INPUT_MODEL pin_model;
    PLAYER_OUTPUT_MODEL pout_model;
    PLAYER_SYNC_MODE psync_mode;
    PLAYER_TEST_PATTERN ptest_pattern;
} PLAYER_PARAM;

extern uint16_t App_Player_getDuty(uint16_t *pu16duty, PLAYER_INPUT_MODEL emodel);

extern PLAYER_RET App_Player_setDuty(uint16_t *pu16duty, uint16_t duty_size, PLAYER_OUTPUT_MODEL emodel);

extern PLAYER_RET App_Player_prepareTestPattern(uint16_t *pu16tpbuf, uint16_t duty_size, PLAYER_TEST_PATTERN eptp);

extern PLAYER_RET App_Player_printDuty(uint16_t *pu16duty, uint16_t col, uint16_t row);

extern PLAYER_RET App_Player_setWorkParam(PLAYER_PARAM *param);

extern PLAYER_RET App_Player(PLAYER_PARAM *param);

#endif /* APP_APP_PLAYER_H_ */
