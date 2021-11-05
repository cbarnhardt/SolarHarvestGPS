/*
 * Abstraction.h
 *
 *  Created on: Jan 20, 2021
 *      Author: jesse
 */

#ifndef ABSTRACTION_H_
#define ABSTRACTION_H_

#include <math.h>



#define     CS_KEY  0x695A
/***********************************************************
 * BLE MODULE UART COMMS DEFINITIONS
 * Module Used: EUSCI_A3
 * RXD: P9.6
 * TXD: P9.7
 ***********************************************************/
// P9.6 as RXD
#define     BLE_UART_RXD_PIN              BIT6
#define     BLE_UART_RXD_POUT             (P9->OUT)
#define     BLE_UART_RXD_PDIR             (P9->DIR)
#define     BLE_UART_RXD_PSEL0            (P9->SEL0)
#define     BLE_UART_RXD_PSEL1            (P9->SEL1)

// P9.7 as TXD
#define     BLE_UART_TXD_PIN              BIT7
#define     BLE_UART_TXD_POUT             (P9->OUT)
#define     BLE_UART_TXD_PDIR             (P9->DIR)
#define     BLE_UART_TXD_PSEL0            (P9->SEL0)
#define     BLE_UART_TXD_PSEL1            (P9->SEL1)

// BLE communication uses EUSCI_A3
#define     BLE_UART_CONFIG_RST          BIT0

#define     BLE_UART_SET_BRCLK_UCLK      0x0000
#define     BLE_UART_SET_BRCLK_ACLK      BIT6
#define     BLE_UART_SET_BRCLK_SMCLK     (BIT6 | BIT7)
#define     BLE_UART_SET_ASYNCH          0x0000
#define     BLE_UART_SET_SYNCH           BIT8
#define     BLE_UART_SET_MODE            0x0000
#define     BLE_UART_SET_ONE_STOP        0x0000
#define     BLE_UART_SET_TWO_STOP        BIT11
#define     BLE_UART_SET_CHAR_LENGTH_7   0x0000
#define     BLE_UART_SET_CHAR_LENGTH_8   0x0000
#define     BLE_UART_SET_LSB_FIRST       0x0000
#define     BLE_UART_SET_MSB_FIRST       BIT13
#define     BLE_UART_SET_ODD_PAR         0x0000
#define     BLE_UART_SET_EVEN_PAR        BIT14
#define     BLE_UART_SET_EN_PAR          BIT15
#define     BLE_UART_RX_IRQ_ENABLE       BIT0

#define     BLE_UART_RX_IRQ_FLAG     BIT0
#define     BLE_UART_TXD_IFG         BIT1

#define     BLE_UART_CTL             (EUSCI_A3->CTLW0)
#define     BLE_UART_BRW             (EUSCI_A3->BRW)
#define     BLE_UART_MCTLW           (EUSCI_A3->MCTLW)
#define     BLE_UART_IFG             (EUSCI_A3->IFG)
#define     BLE_UART_IE              (EUSCI_A3->IE)
#define     BLE_UART_TXD             (EUSCI_A3->TXBUF)
#define     BLE_UART_RXD             (EUSCI_A3->RXBUF)
#define     BLE_UART_IRQn            EUSCIA3_IRQn
#define     BLE_UART_IRQHandler      EUSCIA3_IRQHandler

/***********************************************************
 * PC UART COMMS DEFINITIONS
 * Module Used: EUSCI_A0
 * RXD: P1.2
 * TXD: P1.3
 ***********************************************************/
// P1.2 as RXD
#define     PC_UART_RXD_PIN              BIT2
#define     PC_UART_RXD_POUT             (P1->OUT)
#define     PC_UART_RXD_PDIR             (P1->DIR)
#define     PC_UART_RXD_PSEL0            (P1->SEL0)
#define     PC_UART_RXD_PSEL1            (P1->SEL1)

// P1.3 as TXD
#define     PC_UART_TXD_PIN              BIT3
#define     PC_UART_TXD_POUT             (P1->OUT)
#define     PC_UART_TXD_PDIR             (P1->DIR)
#define     PC_UART_TXD_PSEL0            (P1->SEL0)
#define     PC_UART_TXD_PSEL1            (P1->SEL1)

// PC communication uses EUSCI_A0
#define     PC_UART_CONFIG_RST          BIT0

#define     PC_UART_SET_BRCLK_UCLK      0x0000
#define     PC_UART_SET_BRCLK_ACLK      BIT6
#define     PC_UART_SET_BRCLK_SMCLK     (BIT6 | BIT7)
#define     PC_UART_SET_ASYNCH          0x0000
#define     PC_UART_SET_SYNCH           BIT8
#define     PC_UART_SET_MODE            0x0000
#define     PC_UART_SET_ONE_STOP        0x0000
#define     PC_UART_SET_TWO_STOP        BIT11
#define     PC_UART_SET_CHAR_LENGTH_7   0x0000
#define     PC_UART_SET_CHAR_LENGTH_8   0x0000
#define     PC_UART_SET_LSB_FIRST       0x0000
#define     PC_UART_SET_MSB_FIRST       BIT13
#define     PC_UART_SET_ODD_PAR         0x0000
#define     PC_UART_SET_EVEN_PAR        BIT14
#define     PC_UART_SET_EN_PAR          BIT15
#define     PC_UART_RX_IRQ_ENABLE       BIT0

#define     PC_UART_RX_IRQ_FLAG     BIT0
#define     PC_UART_TXD_IFG         BIT1

#define     PC_UART_CTL             (EUSCI_A0->CTLW0)
#define     PC_UART_BRW             (EUSCI_A0->BRW)
#define     PC_UART_MCTLW           (EUSCI_A0->MCTLW)
#define     PC_UART_IFG             (EUSCI_A0->IFG)
#define     PC_UART_IE              (EUSCI_A0->IE)
#define     PC_UART_TXD             (EUSCI_A0->TXBUF)
#define     PC_UART_RXD             (EUSCI_A0->RXBUF)
#define     PC_UART_IRQn            EUSCIA0_IRQn
#define     PC_UART_IRQHandler      EUSCIA0_IRQHandler

/*
 * TIMER A0 USED FOR STROBING LED
 * TIMER A1 USED FOR TIMING PURPOSES
 */
#define ACLK_SOURCE_FREQ_LFXTCLK        32768
#define TIMER_A_COUNT_1_MSEC            33
#define TIMER_A_COUNT_0_SEC             0
#define TIMER_A_COUNT_1_SEC             32767
#define TIMER_A_COUNT_2_SEC             65535
#define TIMER_A_COUNT_ACTIVITY_ON         1
#define TIMER_A_COUNT_ACTIVITY_OFF        300

#define ACTIVITY_TIMER_A_CTL          (TIMER_A0->CTL)
#define TIMING_TIMER_A_CTL          (TIMER_A1->CTL)
#define TIMER_A_CTL_RST             0x07F7  /*BIT MASK CLEAR THESE BITS TO RST*/
#define TIMER_A_IFG                 BIT0
#define TIMER_A_IRQ_EN              BIT1
#define TIMER_A_COUNT_CLR           BIT2
#define TIMER_A_UP_MODE             BIT4
#define TIMER_A_CONTINOUS_MODE      BIT5
#define TIMER_A_UP_DOWN_MODE        (BIT4 | BIT5)
#define TIMER_A_CLK_DIV_2           BIT6
#define TIMER_A_CLK_DIV_4           BIT7
#define TIMER_A_CLK_DIV_8           (BIT6 | BIT7)
#define TIMER_A_CLK_SEL_ACLK        BIT8
#define TIMER_A_CLK_SEL_SMCLK       BIT9
#define TIMER_A_CLK_SEL_INCLK       (BIT8 | BIT9)

#define TIMER_A_CCTL_RST                0xFDFF /*BIT MASK CLEAR THESE BITS TO RST*/
#define TIMER_A_CCTL_IFG                BIT0
#define TIMER_A_CCTL_CAP_OVERFLOW       BIT1
#define TIMER_A_CCTL_MANUAL_OUT_VAL     BIT2
#define TIMER_A_CCTL_CCI                BIT3
#define TIMER_A_CCTL_IRQ_EN             BIT4
#define TIMER_A_CCTL_OUTMOD_SET         BIT5
#define TIMER_A_CCTL_OUTMOD_TOG_RST     BIT6
#define TIMER_A_CCTL_OUTMOD_SET_RST     (BIT5 | BIT6)
#define TIMER_A_CCTL_OUTMOD_TOG         BIT7
#define TIMER_A_CCTL_OUTMOD_RST         (BIT5 | BIT7)
#define TIMER_A_CCTL_OUTMOD_TOG_SET     (BIT6 | BIT7)
#define TIMER_A_CCTL_OUTMOD_RST_SET     (BIT5 | BIT6 | BIT7)
#define TIMER_A_CCTL_CAP_MODE           BIT8
#define TIMER_A_CCTL_SCCI               BIT9
#define TIMER_A_CCTL_SYNC_CAP_SRC       BIT10
#define TIMER_A_CCTL_CAP_SEL_1          0
#define TIMER_A_CCTL_CAP_SEL_B          BIT11
#define TIMER_A_CCTL_CAP_SEL_GND        BIT12
#define TIMER_A_CCTL_CAP_SEL_VCC        (BIT11 | BIT12)
#define TIMER_A_CCTL_CAP_MODE_RISE      BIT14
#define TIMER_A_CCTL_CAP_MODE_FALL      BIT15
#define TIMER_A_CCTL_CAP_MODE_RISE_FALL (BIT14 | BIT15)

#define ACTIVITY_TIMER_A_CCTL_0       (TIMER_A0->CCTL[0])
#define ACTIVITY_TIMER_A_CCTL_1       (TIMER_A0->CCTL[1])
#define ACTIVITY_TIMER_A_CCTL_2       (TIMER_A0->CCTL[2])
#define ACTIVITY_TIMER_A_CCTL_3       (TIMER_A0->CCTL[3])
#define ACTIVITY_TIMER_A_CCTL_4       (TIMER_A0->CCTL[4])
#define ACTIVITY_TIMER_A_CCTL_5       (TIMER_A0->CCTL[5])

#define TIMING_TIMER_A_CCTL_0       (TIMER_A1->CCTL[0])
#define TIMING_TIMER_A_CCTL_1       (TIMER_A1->CCTL[1])
#define TIMING_TIMER_A_CCTL_2       (TIMER_A1->CCTL[2])
#define TIMING_TIMER_A_CCTL_3       (TIMER_A1->CCTL[3])
#define TIMING_TIMER_A_CCTL_4       (TIMER_A1->CCTL[4])
#define TIMING_TIMER_A_CCTL_5       (TIMER_A1->CCTL[5])

#define ACTIVITY_TIMER_A_CCR_0        (TIMER_A0->CCR[0])
#define ACTIVITY_TIMER_A_CCR_1        (TIMER_A0->CCR[1])
#define ACTIVITY_TIMER_A_CCR_2        (TIMER_A0->CCR[2])
#define ACTIVITY_TIMER_A_CCR_3        (TIMER_A0->CCR[3])
#define ACTIVITY_TIMER_A_CCR_4        (TIMER_A0->CCR[4])
#define ACTIVITY_TIMER_A_CCR_5        (TIMER_A0->CCR[5])

#define TIMING_TIMER_A_CCR_0        (TIMER_A1->CCR[0])
#define TIMING_TIMER_A_CCR_1        (TIMER_A1->CCR[1])
#define TIMING_TIMER_A_CCR_2        (TIMER_A1->CCR[2])
#define TIMING_TIMER_A_CCR_3        (TIMER_A1->CCR[3])
#define TIMING_TIMER_A_CCR_4        (TIMER_A1->CCR[4])
#define TIMING_TIMER_A_CCR_5        (TIMER_A1->CCR[5])

#define ACTIVITY_TIMER_A_EX0        (TIMER_A0->EX0)
#define TIMING_TIMER_A_EX0          (TIMER_A1->EX0)
#define TIMER_A_EX0_PRESCALE_2      BIT0
#define TIMER_A_EX0_PRESCALE_3      BIT1
#define TIMER_A_EX0_PRESCALE_4      (BIT0 | BIT1)
#define TIMER_A_EX0_PRESCALE_5      BIT2
#define TIMER_A_EX0_PRESCALE_6      (BIT0 | BIT2)
#define TIMER_A_EX0_PRESCALE_7      (BIT1 | BIT2)
#define TIMER_A_EX0_PRESCALE_8      (BIT0 | BIT1 | BIT2)

#define ACTIVITY_TIMER_A_0_IRQ_EN     TA0_0_IRQn
#define ACTIVITY_TIMER_A_N_IRQ_EN     TA0_N_IRQn

#define TIMING_TIMER_A_0_IRQ_EN     TA1_0_IRQn
#define TIMING_TIMER_A_N_IRQ_EN     TA1_N_IRQn

#endif /* ABSTRACTION_H_ */
