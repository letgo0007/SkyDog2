/**@file    hal_clock.c
 *
 * MSP430F5xx clock system HAL.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170104 | Yang.Zf | Initial Version
 */

#include "hal_clock.h"

/*****************************************************************************
 * Internal Functions.
 *****************************************************************************/
/*!@brief Set MSP430 VCORE level up.
 * @param level  [1~3] 3 Core level .
 */
void SetVcoreUp(unsigned int level)
{
    // Open PMM registers for write
    PMMCTL0_H = PMMPW_H;
    // Set SVS/SVM high side new level
    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
    // Set SVM low side to new level
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
    // Wait till SVM is settled
    while ((PMMIFG & SVSMLDLYIFG) == 0)
    {
        ;
    }
    // Clear already set flags
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
    // Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;
    // Wait till new level reached
    if ((PMMIFG & SVMLIFG))
    {
        while ((PMMIFG & SVMLVLRIFG) == 0)
        {
            ;
        }
    }
    // Set SVS/SVM low side to new level
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
    // Lock PMM registers for write access
    PMMCTL0_H = 0x00;
}

/*****************************************************************************
 * External Functions.
 *****************************************************************************/
void Clock_init(void)
{

    /*[1] Increase VCORE setting according to CPU frequency.
     *  NOTE: Change core voltage one level at a time..
     */
#if MCLK_F > 8000000   // 1~8MHz
    {
        SetVcoreUp(0x01);
    }
#endif
#if MCLK_F > 16000000  // 8~16MHz
    {
        SetVcoreUp(0x02);
    }
#endif
#if MCLK_F > 20000000  // 16~25MHz
    {
        SetVcoreUp(0x03);
    }
#endif

    /*[2] Select Clock source according to CPU frequency.
     *  DCO     1st source  = REFO   , DIV = 1
     *  MCLK    1st source  = DCODIV , DIV = 1
     *  SMCLK   1st source  = DCODIV , DIV = MCLK/SMCLK
     *  ACLK    1st source  = XT1    , DIV = 1
     *          2nd source  = REFO   , DIV = 1
     */
    UCSCTL3 |= SELREF__REFOCLK;
#if ( XT1_F > 0)
    {
        UCSCTL4 |= SELA__XT1CLK;
    }
#else
    {
        UCSCTL4 |= SELA__REFOCLK;
    }
#endif
    UCSCTL5 = DIVM_0 + DIVS_1 * SMCLK_DIV_BIT + DIVA_0;

    /*[3] Initialize DCO.
     * DCO_F = REFO_F * N * FLLD = MCLK_F
     */
    __bis_SR_register(SCG0);
    UCSCTL0 = 0x0000;
    UCSCTL1 = DCORSEL_7;
    UCSCTL2 = FLLD__1 + MCLK_F / REFO_F;
    __bic_SR_register(SCG0);

    __delay_cycles(782000);

    // Loop until XT1,XT2 & DCO fault flag is cleared
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
        // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;    // Clear fault flags
    } while (SFRIFG1 & OFIFG);                  // Test oscillator fault flag
}


