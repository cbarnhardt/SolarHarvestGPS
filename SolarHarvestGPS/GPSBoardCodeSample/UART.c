/*
 *
 *  Created on: Feb 12, 2021
 *      Author: jesse
 */

#include "UART.h"
#include "Abstraction.h"
#include "stdio.h"

/*
 * Print characters to the serial modules, acts as a functional printf statement, but for UART comms
 */
void printToapp(const char *sentence){
    int i = 0;

    while(sentence[i] != NULL){
        while( !(BLE_UART_TXD_IFG & BLE_UART_IFG) );    //Wait for buffer to be ready for TX
        BLE_UART_TXD = sentence[i];
        while( !(PC_UART_TXD_IFG & BLE_UART_IFG) );
        PC_UART_TXD = sentence[i];
        i++;
    }
}

/*
 * Print characters to the serial modules, used to send the data being saved
 */
void printToapp2(SDcard* Card){
    uint16_t i = 0;
    bool Write_En = 0;
    uint8_t MSGcount = 0;

    while(Write_En == 0){
        if(Card->DataToWrite[i] == '$'){
            Write_En = 1;
            break;
        }
        i++;
    }

    while(i < SD_BLOCK_LENGTH){
        if(Card->DataToWrite[i] == '$'){
            MSGcount += 1;
            printToapp("\n\n");
        }

        while( !(BLE_UART_TXD_IFG & BLE_UART_IFG) );    //Wait for buffer to be ready for TX
        BLE_UART_TXD = Card->DataToWrite[i];
        while( !(PC_UART_TXD_IFG & BLE_UART_IFG) );
        PC_UART_TXD = Card->DataToWrite[i];

        if( (MSGcount == 6) && (Card->DataToWrite[i] == '*') )
            break;

        i++;
    }
    while( !(BLE_UART_TXD_IFG & BLE_UART_IFG) );    //Wait for buffer to be ready for TX
    BLE_UART_TXD = 0x0A;
    while( !(PC_UART_TXD_IFG & BLE_UART_IFG) );
    PC_UART_TXD = 0x0A;
}
