/*
 * SD_Card.h
 *
 *  Created on: Apr 11, 2021
 *      Author: jesse
 */

#ifndef SD_CARD_H_
#define SD_CARD_H_

#include <stdbool.h>
#include "msp432.h"
//#include <string.h>

//SPI SD_CARD REGISTERS
 //Uses UCB1
 //Change this macro to change the usci hardware
    #define SD_CARD_EUSCI             EUSCI_B1
    #define SD_CARD_CTL           (SD_CARD_EUSCI->CTLW0)
    #define SD_CARD_STAT          (SD_CARD_EUSCI->STATW)
    #define SD_CARD_RXBUF         (SD_CARD_EUSCI->RXBUF)
    #define SD_CARD_TXBUF         (SD_CARD_EUSCI->TXBUF)
    #define SD_CARD_CLKDIVIDE     (SD_CARD_EUSCI->BRW)
    #define SD_CARD_IE            (SD_CARD_EUSCI->IE)
    #define SD_CARD_IFG           (SD_CARD_EUSCI->IFG)
    #define SD_CARD_IRQn           EUSCIB1_IRQn
    #define SD_CARD_IRQHandler     EUSCIB1_IRQHandler
    #define SD_CARD_RX_FLAG         BIT0
    #define SD_CARD_TX_FLAG         BIT1

 //SPI GPIO SET UP PORT 6 edit for a different port. Check ph 233 of msp bible for reference
    #define SD_CARD_PORT           P6
    #define SD_CARD_MISO_PIN       BIT5
    #define SD_CARD_MOSI_PIN       BIT4
    #define SD_CARD_CLK_PIN        BIT3
    #define SD_CARD_STE_PIN        BIT2
    #define SD_CARD_CS_PIN         BIT1
    #define SD_CARD_WP_PIN         BIT0


    /*
     * Bit 3 is CLK
     * Bit 4 is MOSI
     * Bit 5 is MISO
     */
    #define SEL0_SD_CARD_MASK      (BIT3 | BIT4 | BIT5)
    #define SEL1_SD_CARD_MASK      0

    #define DUMMY_DATA          0xFF
    /* EACH ADDRESS OF THE SD CARD IS FOR A 512 BYTE BLOCK, 1 BYTE = 8 BITS */
    #define SD_BLOCK_LENGTH     512
    #define BUFF_SIZE           600
    #define CRC7_POLYNOMIAL     0x89

    #define SD_SETTINGS_ADDRESS         0x00000000
    #define SD_DATA_START_ADDRESS       0x00000001
    /*  Found experimentally by seeing which addresses gave "out-of-bounds" error   */
    #define MAX_DATA_ADDRESS            0x03B6FFFF


    /*
     * NMEA RMC message holds 80 chars (80 bytes) to be saved
     *  in a single RMC message. If each block of an SD card is 512 bytes, then
     *          512 / 80 = 6.4
     *  We don't want to split the messages across blocks since it would make it much
     *  more complex to read them back if necessary and make sure no chars are missed.
     *  So we round the value down.
     *          80 * 6 = 480 --> 32 unused bytes of data in a block
     *  The only data that wil lbe saved, however, is from the '$' character to the '*', such that
     *  we are able to save 7 messages in a single block. This will use 503 bytes, meaning only 9 unused bytes
     *  in a byte.
     *  We can find how much position data can be held before rollover occurs:
     *          (2^32 - 1) * 7 = 30 * 10^9 messages (or reads)
     */
    #define MAX_NMEA_RMC_MSG_IN_BLOCK   7

//Clock settings
    #define   CLKPHASEFIRST      0
    #define   CLKPHASESECOND     BITF
    #define   CLKPOLARITY_LOW    0
    #define   CLKPOLARITY_HIGH   BITE

//Bit Settings
    #define   LSBFIRST           0
    #define   MSBFIRST           BITD
    #define   EIGHTBITS          0
    #define   SEVENBITS          BITC

//Command
    #define   SLAVEMODE          0
    #define   MASTERMODE         BITB

 //SPI WIRE MODES //not wire
    #define   THREEWIREMODE          0
    #define   FOURWIREMODE_STEHIGH  BIT9
    #define   FOURWIREMODE_STELOW   BITA
    //#define   I2C               (BIT9|BITA)

//Sync settings
    #define   ASYNC              0
    #define   SYNC               BIT8

 //Clock Select
    #define  ACLK                BIT6
  //#define  SMCLK               BIT7
    #define  SMCLK               BIT7|BIT8

 //STE Mode select
    #define STEPREVCONF          0
    #define STEENSIG             BIT1

//RESET
    #define RST_DISABLE          0
    #define RST_ENABLE           BIT0

//SD Card Commands
    #define COMMAND_START_BITS          0x40
    #define DATA_START_BITS     0xFE
    #define DATA_CRC1           0x11
    #define DATA_CRC2           0x22

    #define NO_CRC              0x01
    #define CMD0                (0x00 | COMMAND_START_BITS)
    #define CMD0_CRC            0x95
    #define CMD1                (0x01 | COMMAND_START_BITS)
    #define CMD1_CRC            0xF9
    #define CMD8                (0x08 | COMMAND_START_BITS)
    #define CMD8_CRC            0x87
    #define CMD9                (0x09 | COMMAND_START_BITS)
    #define CMD9_CRC            0xAF
    #define CMD10               (0x0A | COMMAND_START_BITS)
    #define CMD12               (0x0C | COMMAND_START_BITS)
    #define CMD13               (0x0D | COMMAND_START_BITS)
    #define CMD16               (0x10 | COMMAND_START_BITS)
    #define CMD17               (0x11 | COMMAND_START_BITS)
    #define CMD18               (0x12 | COMMAND_START_BITS)
    #define CMD24               (0x18 | COMMAND_START_BITS)
    #define CMD25               (0x19 | COMMAND_START_BITS)
    #define ACMD41              (0x29 | COMMAND_START_BITS)
    #define ACMD41_CRC          0x77
    #define CMD55               (0x37 | COMMAND_START_BITS)
    #define CMD55_CRC           0x65
    #define CMD58               (0x3A | COMMAND_START_BITS)
    #define CMD58_CRC           0xFD

    #define WRITE_CRC           0x49
    #define READ_CRC            0x39
typedef struct SDcard
{
    unsigned char Setup[SD_BLOCK_LENGTH + 1];
        /*
         * Bytes 0: ADDRESS BYTE 4 OF NEXT AVAILABLE WRITE ADDRESS
         * Bytes 1: ADDRESS BYTE 3 OF NEXT AVAILABLE WRITE ADDRESS
         * Bytes 2: ADDRESS BYTE 2 OF NEXT AVAILABLE WRITE ADDRESS
         * Bytes 3: ADDRESS BYTE 1 OF NEXT AVAILABLE WRITE ADDRESS
         */

    uint32_t ReadAddress;
    unsigned char ReadData[SD_BLOCK_LENGTH + 1];    //Holds read Data for a single block
    unsigned char ReadCRC[2];   //Not used, but SD SPI protocol requires that this is "read"

    uint32_t WriteAddress;
    unsigned char DataToWrite[SD_BLOCK_LENGTH + 1]; //Holds the write data to be put into one block of SD card

    uint32_t NextAvailWriteLocation; //Holds the next available address for data to be written to
} SDcard;

 bool initSDcard(void);
 bool writeToSDcard(SDcard* Card);
 bool readSDcard(SDcard* Card);
 bool SendDummyData(uint8_t amount);
 uint8_t SendSDCommand(uint8_t Command, uint8_t Arg1, uint8_t Arg2, uint8_t Arg3, uint8_t Arg4, uint8_t CRC);
 uint8_t ReadSDcardResponse(void);
#endif /* SD_CARD_H_ */
