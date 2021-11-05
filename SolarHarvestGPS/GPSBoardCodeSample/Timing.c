/*
 * Timing.c
 *
 *  Created on: Jan 23, 2021
 *      Author: jesse
 */

#include <Timing.h>
#include <abstraction.h>

/*
 * Setup one Timer A module to be used for general delays.
 */
void TIMING_TIMER_A_SETUP(void){
    TIMING_TIMER_A_CTL &= ~TIMER_A_CTL_RST;

    TIMING_TIMER_A_CCR_0 = TIMER_A_COUNT_1_SEC;
    TIMING_TIMER_A_CCTL_0 &= ~TIMER_A_CCTL_RST;
    TIMING_TIMER_A_CCTL_0 |= TIMER_A_CCTL_IRQ_EN;

    TIMING_TIMER_A_CTL |=   TIMER_A_COUNT_CLR
                       |    TIMER_A_UP_MODE
                       |    TIMER_A_CLK_SEL_ACLK;
    TIMING_TIMER_A_CTL &= ~TIMER_A_UP_MODE;

    NVIC_EnableIRQ(TIMING_TIMER_A_0_IRQ_EN);
}

/*
 * Setup another Timer A module to be used as the activity timer. This way, can have
 * general delays while still keeping track of overall activity time.
 */
void ACTIVITY_TIMER_A_SETUP(void){
    ACTIVITY_TIMER_A_CTL &= ~TIMER_A_CTL_RST;

    //ACTIVITY_TIMER_A_CCR_0 = 30720; //60 SECONDS
    ACTIVITY_TIMER_A_CCR_0 = 2560;    // 5 SECONDS
    ACTIVITY_TIMER_A_CCTL_0 &= ~TIMER_A_CCTL_RST;
    ACTIVITY_TIMER_A_CCTL_0 |= TIMER_A_CCTL_IRQ_EN;

    ACTIVITY_TIMER_A_CTL |=   TIMER_A_COUNT_CLR
                       |    TIMER_A_UP_MODE
                       |    TIMER_A_CLK_SEL_ACLK
                       |    TIMER_A_CLK_DIV_8;
    ACTIVITY_TIMER_A_EX0 |= TIMER_A_EX0_PRESCALE_8;
    ACTIVITY_TIMER_A_CTL &= ~TIMER_A_UP_MODE;

    NVIC_EnableIRQ(ACTIVITY_TIMER_A_0_IRQ_EN);
}
