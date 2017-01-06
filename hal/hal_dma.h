/**@file    hal_dma.h
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 2017Äê1ÔÂ4ÈÕ | GKX100052 | Initial Version
 */

#ifndef HAL_HAL_DMA_H_
#define HAL_HAL_DMA_H_

#include "msp430.h"
#define DMA_MAX_LLI_SIZE    8

//DMA linked list structure
typedef struct DMA_LLI
{
    unsigned long src_add;      //source address
    unsigned long dst_add;      //destination address
    unsigned int size;          //transfer size
    unsigned int trans_mode;   //transfer mode
    unsigned int trigger;      //transfer trigger
    unsigned long next_lli_add;             //pointer to next lli.
} DMA_LLI;

unsigned char Dma_start(DMA_LLI *lli, unsigned char dma_ch);

#endif /* HAL_HAL_DMA_H_ */
