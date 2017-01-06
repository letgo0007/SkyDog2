/**@file    hal_pwm.c
 *
 * MSP430F5xxx PWM input / output HAL.
 *
 * @history
 * Date     | Author  | Comment
 * ------------------------------------
 * 20170105 | Yang.Zf | Initial Version
 */

#include "hal_pwm.h"

/***[ PWM OUT ] start*******************************************************/

#ifndef PWM_OUT_TIMER_BASE
#define PWM_OUT_TIMER_BASE      TIMER_A2_BASE
#endif

/*****************************************************************************
 * [ PWM OUT ]  macro & register define.
 *****************************************************************************/
#if PWM_OUT_TIMER_BASE == TIMER_A2_BASE
#define SET_PWM_OUT_FUNC_IO       do{\
    P2DIR |= BIT3+BIT4+BIT5;\
    P2SEL |= BIT3+BIT4+BIT5;\
}while(0)
#define PWM_OUT_CTL               TA2CTL
#define PWM_OUT_CCTL0             TA2CCTL0
#define PWM_OUT_CCTL1             TA2CCTL1
#define PWM_OUT_CCTL2             TA2CCTL2
#define PWM_OUT_CCR0              TA2CCR0
#define PWM_OUT_CCR1              TA2CCR1
#define PWM_OUT_CCR2              TA2CCR2
#endif

/*****************************************************************************
 * [ PWM OUT ]  External Functions.
 *****************************************************************************/

void PwmOut_init(void)
{
    SET_PWM_OUT_FUNC_IO
    ;

    // ACLK source , up-down mode, clear TAR;
    PWM_OUT_CTL = TASSEL__ACLK + MC__UPDOWN + TACLR;
    PWM_OUT_CCTL0 = OUTMOD_0;
    PWM_OUT_CCTL1 = OUTMOD_0;
    PWM_OUT_CCTL2 = OUTMOD_0;
}

void PwmOut_setOutput(unsigned char ch, unsigned int freq, unsigned int duty)
{
    switch (ch)
    {
    case 0:
        if (freq)
        {
            PWM_OUT_CCR0 = ACLK_F / freq / 2; // Frequency = PWM Period/2
            PWM_OUT_CCTL0 = OUTMOD_4;          // CCR0 toggle
        }
        else
        {
            PWM_OUT_CCTL0 = OUTMOD_0;          // Disable
        }

        break;
    case 1:
        if (freq)
        {
            PWM_OUT_CCR0 = ACLK_F / freq / 2; // Frequency = PWM Period/2
            PWM_OUT_CCR1 = (unsigned long) PWM_OUT_CCR0 * duty / 256; // Duty
            PWM_OUT_CCTL1 = OUTMOD_6;          // CCR1 toggle/set
        }
        else
        {
            PWM_OUT_CCTL1 = OUTMOD_0;          // Disable
        }

        break;
    case 2:
        if (freq)
        {
            PWM_OUT_CCR0 = ACLK_F / freq / 2; // Frequency = PWM Period/2
            PWM_OUT_CCR2 = (unsigned long) PWM_OUT_CCR0 * (256 - duty) / 256;   // Duty
            PWM_OUT_CCTL2 = OUTMOD_6;          // CCR2 toggle/set
        }
        else
        {
            PWM_OUT_CCTL2 = OUTMOD_0;          // Disable
        }
        break;
    default:
        break;
    }
}

/***[ PWM IN ] start*******************************************************/

#ifndef PWM_IN_TIMER_BASE
#define PWM_IN_TIMER_BASE       TIMER_A1_BASE
#endif

/*****************************************************************************
 * [ PWM IN ]  macro & register define.
 *****************************************************************************/
#if PWM_IN_TIMER_BASE == TIMER_A1_BASE
#define SET_PWM_IN_FUNC_IO       do{\
    P2DIR &= ~(BIT0 + BIT1);\
    P2SEL |= BIT0 + BIT1;\
}while(0)
#define PWM_IN_VECTOR0           TIMER1_A0_VECTOR
#define PWM_IN_VECTOR1           TIMER1_A1_VECTOR
#define PWM_IN_CTL               TA1CTL
#define PWM_IN_CCTL0             TA1CCTL0
#define PWM_IN_CCTL1             TA1CCTL1
#define PWM_IN_CCTL2             TA1CCTL2
#define PWM_IN_CCR0              TA1CCR0
#define PWM_IN_CCR1              TA1CCR1
#define PWM_IN_CCR2              TA1CCR2
#endif

/*****************************************************************************
 * [ PWM IN ]  operation buffers.
 *****************************************************************************/
static unsigned int Pwm_In_EdgeCount[3];
static unsigned int Pwm_In_Freq[3];

/*****************************************************************************
 * [ PWM IN ]  Internal Functions.
 *****************************************************************************/
// TA1.0 ISR
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PWM_IN_VECTOR0
__interrupt void PwmIn_Isr_0(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PWM_IN_VECTOR0))) PwmIn_Isr_0 (void)
#else
#error Compiler not supported!
#endif
{
    unsigned char i;

    for (i = 0; i < 3; i++)
    {
        Pwm_In_Freq[i] = Pwm_In_EdgeCount[i] / 2;
        Pwm_In_EdgeCount[i] = 0;
    }
}

// TA1.1 ~ TA1.2 ISR
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PWM_IN_VECTOR1
__interrupt void PwmIn_Isr(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PWM_IN_VECTOR1))) PwmIn_Isr (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(TA1IV, 14))
    {
    case 0:
        break;
    case 2:          //TA1.1 P2.0
        Pwm_In_EdgeCount[1]++;
        break;
    case 4:          //TA1.2 P2.1
        Pwm_In_EdgeCount[2]++;
        break;
    case 6:
        break;
    case 8:
        break;
    case 10:
        break;
    case 12:
        break;
    case 14:          //Over Flow
        break;
    default:
        break;
    }
}
/*****************************************************************************
 * [ PWM IN ]  External Functions.
 *****************************************************************************/
void PwmIn_init(void)
{
    SET_PWM_IN_FUNC_IO
    ;
    // ACLK, up mode , clear TAR;
    PWM_IN_CTL = TASSEL_1 + MC_1 + TACLR + TAIE;

    // CCR0 is used to set a 1s cycle to calculate PWM frequency & duty.
    PWM_IN_CCTL0 = CCIE;
    PWM_IN_CCR0 = 0x10000 / ACLK_F;

    /* CCR1 & 2 is used to capture edges.
     * Capture on both edge.
     * Synchronous capture.
     * Capture Mode.
     * Enable interrupt.
     */
    PWM_IN_CCTL1 = CM_3 + SCS + CAP + CCIE;
    PWM_IN_CCTL2 = CM_3 + SCS + CAP + CCIE;
}

unsigned int PwmIn_getFreq(unsigned char ch)
{
    return Pwm_In_Freq[ch];
}

unsigned int PwmIn_getDuty(unsigned char ch)
{
    return 0;          //TODO : not available yet.
}

