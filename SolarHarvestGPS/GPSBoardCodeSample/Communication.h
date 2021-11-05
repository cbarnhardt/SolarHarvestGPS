/*
 * Communication.h
 *
 *  Created on: Jan 20, 2021
 *      Author: jesse
 */

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <msp432.h>

// PC communication uses SMCLK (unmodified) for baud rate
#define     PC_UART_SMCLK_FREQ        (3*pow(10, 6))
#define     PC_SERIAL_BAUD_RATE       9600

#define     BLE_UART_SMCLK_FREQ        (3*pow(10, 6))
#define     BLE_SERIAL_BAUD_RATE       9600


void PC_2_MSP_COMM_SETUP(void);
    void PC_2_MSP_TX_RX_PIN_SETUP(void);
    void PC_2_MSP_UART_INTERFACE_SETUP(void);
void BLE_2_MSP_COMM_SETUP(void);
        void BLE_2_MSP_TX_RX_PIN_SETUP(void);
        void BLE_2_MSP_UART_INTERFACE_SETUP(void);
short BRS_Lookup(float frac);
#endif /* COMMUNICATION_H_ */
