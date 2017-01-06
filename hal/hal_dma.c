/**@file    hal_dma.c
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 2017Äê1ÔÂ4ÈÕ | GKX100052 | Initial Version
 */

#include "hal_dma.h"
#include "stdio.h"
#include "board.h"

static unsigned long g_Dma0_Lli_Queue[DMA_MAX_LLI_SIZE];

unsigned char Dma_start(DMA_LLI *lli, unsigned char dma_ch)
{
    //Check NULL pointer & invalid size.
    if ((lli->src_add == 0) || (lli->src_add == 0) || (lli->size == 0))
    {
        return 0;
    }

    //Set DMA according to channel.
    switch (dma_ch)
    {
    case 0: //DMA channel 0.
    {
        if ((DMA0SZ != 0) && (DMA0CTL & DMAEN)) //If DMA is busy , add LLI to queue.
        {
            //find empty next LLI.
            unsigned char i = 0;
            while (g_Dma0_Lli_Queue[i] != 0)
            {
                i++;
                if (i >= DMA_MAX_LLI_SIZE) //DMA LLI queue is full
                {
                    return 0;
                }
            }
            //Copy LLI address to empty Queue.
            g_Dma0_Lli_Queue[i] = (unsigned long) &lli;
        }
        else // DMA is free , start transfer.
        {
            DMACTL0 &= 0xFF00;
            DMACTL0 |= lli->trigger;
            __data16_write_addr((unsigned short) &DMA0SA, (unsigned long) lli->src_add);
            __data16_write_addr((unsigned short) &DMA0DA, (unsigned long) lli->dst_add);
            DMA0SZ = lli->size;
            DMA0CTL = lli->trans_mode;
            g_Dma0_Lli_Queue[0] = lli->next_lli_add;
            DMA0CTL |= DMAIE + DMAEN;
        }

        break;
    }
    case 1: //DMA channel 1.
    {
        DMACTL0 &= 0x00FF;
        DMACTL0 |= lli->trigger;
        __data16_write_addr((unsigned short) &DMA1SA, (unsigned long) lli->src_add);
        __data16_write_addr((unsigned short) &DMA1DA, (unsigned long) lli->dst_add);
        DMA1SZ = lli->size;
        DMA1CTL = lli->trans_mode;
        break;
    }
    case 2: //DMA channel 2.
    {
        DMACTL1 &= 0xFF00;
        DMACTL1 |= lli->trigger;
        __data16_write_addr((unsigned short) &DMA0SA, (unsigned long) lli->src_add);
        __data16_write_addr((unsigned short) &DMA0DA, (unsigned long) lli->dst_add);
        DMA2SZ = lli->size;
        DMA2CTL = lli->trans_mode;
        break;
    }
    default:
        return 0;
    }

    return 1;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=DMA_VECTOR
__interrupt void Dma_Isr(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(DMA_VECTOR))) Dma_Isr (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(DMAIV, 16))
    {
    case 0:
        break;
    case 2: // DMA0IFG = DMA Channel 0
        if (g_Dma0_Lli_Queue[0] != 0) //If there is a next LLI task.
        {
            Dma_start(g_Dma0_Lli_Queue[0], 0);
            unsigned char i = 0;
            for (i = 0; i < DMA_MAX_LLI_SIZE - 1; i++)
            {
                g_Dma0_Lli_Queue[i] = g_Dma0_Lli_Queue[i + 1];
            }
            g_Dma0_Lli_Queue[DMA_MAX_LLI_SIZE - 1] = 0;
        }
        break;
    case 4: // DMA1IFG = DMA Channel 1
        break;
    case 6: // DMA2IFG = DMA Channel 2
        break;
    case 8:
        break;
    case 10:
        break;
    case 12:
        break;
    case 14:
        break;
    case 16:
        break;
    default:
        break;
    }
}
