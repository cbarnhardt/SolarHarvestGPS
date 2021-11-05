/*
 * Process.h
 *
 *  Created on: Feb 12, 2021
 *      Author: jesse
 */

#ifndef UART_H_
#define UART_H_

#include    "msp.h"
#include    "stdbool.h"
#include    "SD_Card.h"

#define     IDLE            0x00
#define     STORE_CMD       0x10
#define     DIR_CMD         0x11
#define     MEM_CMD         0x12
#define     READ_CMD        0x13
#define     DELETE_CMD      0x14
#define     ERROR_FLAG      0x15
#define     CLEAR_CMD       0x16

#define     ACK_SIGNAL      0x06
#define     NEW_LINE        0x0A
#define     EOT_SIGNAL      0x04

void printToapp(const char *sentence);
void printToapp2(SDcard* Card);

#endif /* UART_H_ */
