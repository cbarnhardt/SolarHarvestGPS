/*
 * Communication.c
 *
 *  Created on: Jan 20, 2021
 *      Author: jesse
 */

#include "Communication.h"
#include "Abstraction.h"


/*
 * Communication setup between PC console terminal and MCU
 */
void PC_2_MSP_COMM_SETUP(void){
    PC_2_MSP_TX_RX_PIN_SETUP();
    PC_2_MSP_UART_INTERFACE_SETUP();
}

/*
 * Communication setup between MCU and BLE module
 */
void BLE_2_MSP_COMM_SETUP(void){
    BLE_2_MSP_TX_RX_PIN_SETUP();
    BLE_2_MSP_UART_INTERFACE_SETUP();
}

/*
 * Set up 1.2 for UART RXD and 1.3 for UART TXD, both for eUSCI-> UCA0
 */
void PC_2_MSP_TX_RX_PIN_SETUP(void){
    PC_UART_RXD_PSEL0 |= PC_UART_RXD_PIN;
    PC_UART_RXD_PSEL1 &= ~PC_UART_RXD_PIN;

    PC_UART_TXD_PSEL0 |= PC_UART_TXD_PIN;
    PC_UART_TXD_PSEL1 &= ~PC_UART_TXD_PIN;
}

/*
 * Set up 9.6 for UART RXD AND 0.7 FOR UART TXD, both for eUSCI-> UCA3
 */
void BLE_2_MSP_TX_RX_PIN_SETUP(void){
    BLE_UART_RXD_PSEL0 |= BLE_UART_RXD_PIN;
    BLE_UART_RXD_PSEL1 &= ~BLE_UART_RXD_PIN;

    BLE_UART_TXD_PSEL0 |= BLE_UART_TXD_PIN;
    BLE_UART_TXD_PSEL1 &= ~BLE_UART_TXD_PIN;
}

/*
 * Set up EUSCIA0 for serial communications between PC and MSP
 */
void PC_2_MSP_UART_INTERFACE_SETUP(void){
    double  N = 0;
    int     BRF = 0;
    short   BRS = 0;

    PC_UART_CTL = PC_UART_CONFIG_RST;   /* Clear past configs and hold eUSCIA0
                                           module under software reset during new config */
    PC_UART_MCTLW &= ~0xFFF1;       // Prepare Modulation Control Word Register for Config

    PC_UART_CTL |=  PC_UART_SET_BRCLK_SMCLK
                |   PC_UART_SET_ASYNCH
                |   PC_UART_SET_MODE
                |   PC_UART_SET_ONE_STOP
                |   PC_UART_SET_CHAR_LENGTH_8
                |   PC_UART_SET_LSB_FIRST;

    N = PC_UART_SMCLK_FREQ / PC_SERIAL_BAUD_RATE;

    if(N>16){
        PC_UART_BRW = (int)(N/16);    // Set baud rate prescaler
        PC_UART_MCTLW |= BIT0;      // Oversampling is recommended, enable it

        BRF = (int)(fabs( (N/16) - ((int)(N/16)) ) * 16);
        PC_UART_MCTLW |= (BRF << 4);

        BRS = BRS_Lookup( ((float)BRF)/16 );
        PC_UART_MCTLW |= (BRS << 8);
    }
    else{
        PC_UART_BRW = (int)(N);
        PC_UART_MCTLW &= ~BIT0;      // Oversampling not needed

        BRS = BRS_Lookup((N - (int)N));
        PC_UART_MCTLW |= (BRS << 8);
    }

    PC_UART_CTL &= ~PC_UART_CONFIG_RST;  // Take the EUSCIA0 out of software reset and into action
}

/*
 * Set up EUSCIA3 for serial communications between MSP and BLE module
 */
void BLE_2_MSP_UART_INTERFACE_SETUP(void){
    double  N = 0;
    int     BRF = 0;
    short   BRS = 0;

    BLE_UART_CTL = BLE_UART_CONFIG_RST;   /* Clear past configs and hold eUSCIA3
                                           module under software reset during new config */
    BLE_UART_MCTLW &= ~0xFFF1;       // Prepare Modulation Control Word Register for Config

    BLE_UART_CTL |=  BLE_UART_SET_BRCLK_SMCLK
                |   BLE_UART_SET_ASYNCH
                |   BLE_UART_SET_MODE
                |   BLE_UART_SET_ONE_STOP
                |   BLE_UART_SET_CHAR_LENGTH_8
                |   BLE_UART_SET_LSB_FIRST;

    N = BLE_UART_SMCLK_FREQ / BLE_SERIAL_BAUD_RATE;

    if(N>16){
        BLE_UART_BRW = (int)(N/16);    // Set baud rate prescaler
        BLE_UART_MCTLW |= BIT0;      // Oversampling is recommended, enable it

        BRF = (int)(fabs( (N/16) - ((int)(N/16)) ) * 16);
        BLE_UART_MCTLW |= (BRF << 4);

        BRS = BRS_Lookup( ((float)BRF)/16 );
        BLE_UART_MCTLW |= (BRS << 8);
    }
    else{
        BLE_UART_BRW = (int)(N);
        BLE_UART_MCTLW &= ~BIT0;      // Oversampling not needed

        BRS = BRS_Lookup((N - (int)N));
        BLE_UART_MCTLW |= (BRS << 8);
    }

    BLE_UART_CTL &= ~PC_UART_CONFIG_RST;  // Take the EUSCIA0 out of software reset and into action
}

/*
 * This is a look up funciton, table taken from MSP documentation
 */
short BRS_Lookup(float frac){
    float Frac_Port[] = {0.0000, 0.0529, 0.0715, 0.0835, 0.1001, 0.1252, 0.1430,
                         0.1670, 0.2147, 0.2224, 0.2503, 0.3000, 0.3335, 0.3575,
                         0.3753, 0.4003, 0.4286, 0.4378, 0.5002, 0.5715, 0.6003,
                         0.6254, 0.6432, 0.6667, 0.7001, 0.7147, 0.7503, 0.7861,
                         0.8004, 0.8333, 0.8464, 0.8572, 0.8751, 0.9004, 0.9170,
                         0.9288};
    float BRS_Table[] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x11, 0x21, 0x22,
                         0x44, 0x25, 0x49, 0x4A, 0x52, 0x92, 0x53, 0x55, 0xAA, 0x6B,
                         0xAD, 0xB5, 0xB6, 0xD6, 0xB7, 0xBB, 0xDD, 0xED, 0xEE, 0xBF,
                         0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};
    int temp = 0;

    float diff_new = 0;
    float diff_old = fabs(frac - Frac_Port[0]);

    int i = 0;
    int length = sizeof(Frac_Port) / sizeof(Frac_Port[0]);

    for(i = 1; i < length; i++){
        diff_new = fabs(frac - Frac_Port[i]);
        if(diff_old > diff_new)
            temp = i;
        diff_old = diff_new;
    }

    return BRS_Table[temp];
}

