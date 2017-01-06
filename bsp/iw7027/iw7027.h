/**@file    iw7027.h
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 2017Äê1ÔÂ6ÈÕ | GKX100052 | Initial Version
 */

#ifndef BSP_IW7027_IW7027_H_
#define BSP_IW7027_IW7027_H_

#include "stdint.h"

#define IW_CH_COL              6
#define IW_CH_ROW              10
#define IW_CH_AMOUNT           IW_CH_COL*IW_CH_ROW

#define IW7027_DEV_AMOUNT       4
#define IW7027_CH_PER_DEV       16
#define IW7027_DAISY_CHAIN      0

#define IW_SEL_0                (0x0001)
#define IW_SEL_1                (0x0002)
#define IW_SEL_2                (0x0004)
#define IW_SEL_3                (0x0008)
//#define IW_SEL_4                (0x0010)
//#define IW_SEL_5                (0x0020)
//#define IW_SEL_6                (0x0040)
//#define IW_SEL_7                (0x0080)
#define IW_SEL_ALL              (IW_SEL_0 + IW_SEL_1 + IW_SEL_2  +IW_SEL_3 /*+ IW_SEL_4 + IW_SEL_5 + IW_SEL_6  +IW_SEL_7*/ )

static const uint16_t IW_SEL_LIST[IW7027_DEV_AMOUNT] =
{
IW_SEL_0, IW_SEL_1, IW_SEL_2, IW_SEL_3
//IW_SEL_4 ,IW_SEL_5, IW_SEL_6, IW_SEL_7,
        };

typedef enum IW7027_RET
{
    IW7027_FAIL = 0x00, IW7027_SUCCESS = 0x01
} IW7027_RET;

typedef struct tIw7027_InitParam
{
    uint16_t CH_EN[IW7027_DEV_AMOUNT];
} tIw7027_InitParam;

IW7027_RET Iw7027_puts(uint16_t iw_sel, uint8_t reg, uint8_t len, uint8_t *pu8data);

IW7027_RET Iw7027_gets(uint16_t iw_sel, uint8_t reg, uint8_t len, uint8_t *pu8data);

void Iw7027_putc(uint16_t iw_sel, uint8_t reg, uint8_t u8data);

uint8_t Iw7027_getc(uint16_t iw_sel, uint8_t reg);

IW7027_RET Iw7027_checkGetWithTimeOut(uint16_t iw_sel, uint8_t reg, uint8_t compare_val, uint8_t compare_mask);

IW7027_RET Iw7027_init(uint8_t bon, uint8_t *init_map);

IW7027_RET Iw7027_setDuty(uint16_t *duty, uint8_t *sort_map);

IW7027_RET Iw7027_setCurrent(uint8_t current);

IW7027_RET Iw7027_setFreq(uint8_t freq, uint8_t n);

#endif /* BSP_IW7027_IW7027_H_ */
