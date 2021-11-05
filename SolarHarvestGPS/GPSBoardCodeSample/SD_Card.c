/*
 * SD_Card.c
 *
 *  Created on: Apr 11, 2021
 *      Author: jesse
 */

#include "SD_Card.h"
#include <stdbool.h>
#include "msp432.h"
#include <string.h>


/*
 * Configure one Eusci port for SPI communications with SD card.
 *
 */
 bool initSDcard(void)
 {
     uint8_t CommandResponse = 0;

     SD_CARD_CTL = RST_ENABLE;
     SD_CARD_CTL =  (CLKPHASEFIRST    |
                  CLKPOLARITY_HIGH |
                  MSBFIRST         |
                  EIGHTBITS        |
                  MASTERMODE       |
                  THREEWIREMODE    |
                  SYNC             |
                  ACLK             |
                  STEPREVCONF);

     SD_CARD_CTL &= ~RST_ENABLE;    //Undo the Software reset

    SD_CARD_CLKDIVIDE = 1;

    //Set CS pin as GPIO output
    (SD_CARD_PORT->SEL0) &= ~(SD_CARD_CS_PIN);
    (SD_CARD_PORT->SEL1) &= ~(SD_CARD_CS_PIN);
    (SD_CARD_PORT->DIR)  |= (SD_CARD_CS_PIN);

    //These are the euscib pins, set them to their secondary functions
    (SD_CARD_PORT->SEL0) |= SEL0_SD_CARD_MASK;
    (SD_CARD_PORT->SEL1) |= 0;

    (SD_CARD_PORT->OUT) |= (SD_CARD_CS_PIN); //make sure the CS pin is high

    /* Begin communication initialization process for SD card
     * Must follow this exact process, as SD card relies upon this
     * process to properly perform SPI comms.
     * See more information on SD card initialization at:
     *  http://elm-chan.org/docs/mmc/mmc_e.html
     */
    (SD_CARD_PORT->OUT) &= ~SD_CARD_CS_PIN;
    SendDummyData(100);

    //CommandResponse = SendSDCommand(CMD0, 0, 0, 0, 0, CMD0_CRC);
    if(SendSDCommand(CMD0, 0, 0, 0, 0, CMD0_CRC) != 1)  //Idle means good
        return false;
    //CommandResponse = 0;

    //CommandResponse = SendSDCommand(CMD8, 0, 0, 1, 0xAA, CMD8_CRC);
    if(SendSDCommand(CMD8, 0, 0, 1, 0xAA, CMD8_CRC) != 1)   //Idle means good
        return false;
    //CommandResponse = 0;

    //CommandResponse = SendSDCommand(CMD55, 0, 0, 0, 0, NO_CRC);
    if(SendSDCommand(CMD55, 0, 0, 0, 0, NO_CRC) != 1)   //idle means good
        return false;
    //CommandResponse = 0;

    //CommandResponse = SendSDCommand(ACMD41, 0x40, 0, 0, 0, ACMD41_CRC);
    if(SendSDCommand(ACMD41, 0x40, 0, 0, 0, ACMD41_CRC) != 1)   //idle means good
        return false;
    //CommandResponse = 0;

    CommandResponse = SendSDCommand(CMD58, 0, 0, 0, 0, NO_CRC);
    if( (CommandResponse != 5) && (CommandResponse != 1))   //idle means good
        return false;
    CommandResponse = 0;

    /* If process is done correctly, SD card will return 0 once it is ready
     * to operate in SPI mode normally.
     */
    while( SendSDCommand(CMD1, 0, 0, 0, 0, CMD1_CRC) );

    (SD_CARD_PORT->OUT) |= SD_CARD_CS_PIN;
    return true;
 }

 /*
  * Sends dummy data to SD card. Done prior to initialization process
  * so that SD card interface module knows what clock frequency to expect
  * for SD card initialization process. Also used prior to sending commands
  * as recommended through guides.
  */
 bool SendDummyData(uint8_t amount){
     uint8_t i = 0;

     for(i = 0; i < amount; i++){
         while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0 ); //Wait until ready for new Data to be sent
             SD_CARD_EUSCI->TXBUF = DUMMY_DATA;
     }
     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0 );

     return true;
 }

 /*
  * Takes in command information and sends it in a packet to the SD card.
  * Packet formation follows standard setup
  */
 uint8_t SendSDCommand(uint8_t Command, uint8_t Arg1, uint8_t Arg2, uint8_t Arg3, uint8_t Arg4, uint8_t CRC){
     uint8_t Response = 0xFF;
     uint8_t i = 0;
     unsigned char Addtional_Response[5] = {' ', ' ', ' ', ' '};

     //(SD_CARD_PORT->OUT) &= ~SD_CARD_CS_PIN;    //SD Card CS is active low

     SendDummyData(1);

     //Send first byte: COMMAND
     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0 );
     SD_CARD_EUSCI->TXBUF = Command;

     //Send second byte: ARGUEMENT 1
     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0 );
     SD_CARD_EUSCI->TXBUF = Arg1;

     //Send third byte: ARGUEMENT 2
     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0 );
     SD_CARD_EUSCI->TXBUF = Arg2;

     //Send fourth byte: Argument 3
     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
     SD_CARD_EUSCI->TXBUF = Arg3;

     //Send fifth byte: Argument 4
     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
     SD_CARD_EUSCI->TXBUF = Arg4;

     //Send sixth and final byte: CRC + 1 because of SD command format std
     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
     SD_CARD_EUSCI->TXBUF = CRC;

     //Wait for transimssion of final byte
     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);

     Response = SD_CARD_EUSCI->RXBUF; //Read buffer to ensure flag is cleared

     while( (Response == 0xFF)) //SD card will send all high bits until command is executed
         Response = ReadSDcardResponse();

     /*
      * These commands result in an additional 32 bits (4 bytes)
      * of response from SD card. These must be read to ensure proper
      * communication behaviour, even if they are not used.
      */
     if( (Command == CMD8) || (Command == CMD58) ){
         (SD_CARD_PORT->OUT) &= ~SD_CARD_CS_PIN;
         for(i = 0; i < 4; i++){
             while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
             SD_CARD_EUSCI->TXBUF = 0xFF;  //Send no data, but trigger clk cycles

             while( (SD_CARD_EUSCI->IFG & SD_CARD_RX_FLAG) == 0);   //Wait for char to be received
             Addtional_Response[i] = SD_CARD_EUSCI->RXBUF;
         }
     }

     return Response;
 }

 /*
  * Read the MISO data line. Send CLK pulse by sending dummy data
  * to allow SD card to respond
  */
 uint8_t ReadSDcardResponse(void){
     uint8_t temp = SD_CARD_EUSCI->RXBUF; //Read buffer to ensure flag is cleared
     temp = 0;

     (SD_CARD_PORT->OUT) &= ~SD_CARD_CS_PIN;    //SD Card CS is active low

     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
     SD_CARD_EUSCI->TXBUF = 0xFF;  //Send no data, but trigger clk cycles

     while( (SD_CARD_EUSCI->IFG & SD_CARD_RX_FLAG) == 0);   //Wait for char to be received
     temp = SD_CARD_EUSCI->RXBUF;

     return temp;
 }

 /*
  * Writes data to SD card. Relies on address setup prior to calling
  * function.
  */
 bool writeToSDcard(SDcard* Card){
     uint16_t i = 0;

     /*
      * These variables were used in troubleshooting SD card communications
      * during development
      */
     uint8_t WriteStartResponse = 0xFF;
     uint8_t DataResponse = 0x00;

     uint8_t Address_Byte_1 = (Card->WriteAddress & 0xFF000000 ) >> 24;
     uint8_t Address_Byte_2 = (Card->WriteAddress & 0x00FF0000 ) >> 16;
     uint8_t Address_Byte_3 = (Card->WriteAddress & 0x0000FF00 ) >> 8;
     uint8_t Address_Byte_4 = (Card->WriteAddress & 0x000000FF);

     (SD_CARD_PORT->OUT) &= ~SD_CARD_CS_PIN;    //SD Card CS is active low
     WriteStartResponse = SendSDCommand(CMD24, Address_Byte_1, Address_Byte_2, Address_Byte_3, Address_Byte_4, READ_CRC);

     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
     SD_CARD_EUSCI->TXBUF = DATA_START_BITS;

     for(i = 0; i < SD_BLOCK_LENGTH; i++){
         while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0 );
         SD_CARD_EUSCI->TXBUF = Card->DataToWrite[i];
     }

     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
          SD_CARD_EUSCI->TXBUF = DATA_CRC1;

     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
          SD_CARD_EUSCI->TXBUF = DATA_CRC2;

     DataResponse = SD_CARD_EUSCI->RXBUF;

     while( !(  ((DataResponse & BIT3) == 0) && ((DataResponse & BIT0) == 1)) ) // Zero means busy, will go 0xFF when done
      {
          DataResponse = ReadSDcardResponse();
      }

     DataResponse = 0;

     while( DataResponse != 0xFF ) // Zero means busy, will go 0xFF when done
     {
         DataResponse = ReadSDcardResponse();
     }


     (SD_CARD_PORT->OUT) |= SD_CARD_CS_PIN;
     return true;
 }

 /*
  * Reads data from SD card. Relies on address setup prior to calling
  * function.
  */
 bool readSDcard(SDcard* Card)
{
    uint16_t i = 0;

     /*
      * These variables were used in troubleshooting SD card communications
      * during development
      */
    uint8_t ReadStartResponse = 0xFF;
    uint8_t DataResponse = 0x00;

    uint8_t Address_Byte_1 = (Card->ReadAddress & 0xFF000000 ) >> 24;
    uint8_t Address_Byte_2 = (Card->ReadAddress & 0x00FF0000 ) >> 16;
    uint8_t Address_Byte_3 = (Card->ReadAddress & 0x0000FF00 ) >> 8;
    uint8_t Address_Byte_4 = (Card->ReadAddress & 0x000000FF);

    (SD_CARD_PORT->OUT) &= ~SD_CARD_CS_PIN;    //SD Card CS is active low
    ReadStartResponse = SendSDCommand(CMD17, Address_Byte_1, Address_Byte_2, Address_Byte_3, Address_Byte_4, READ_CRC);

    DataResponse = SD_CARD_EUSCI->RXBUF;

    while( (DataResponse != DATA_START_BITS) || ((DataResponse & (BIT7 | BIT6 | BIT5 | BIT4) ) == 0) ) // Zero means busy, will go 0xFF when done
    {
        DataResponse = ReadSDcardResponse();
        if( (DataResponse == 0X08) )
            return false;   //Address was out of range
    }

    for(i = 0; i < SD_BLOCK_LENGTH; i++){
         while( (SD_CARD_EUSCI->IFG & SD_CARD_RX_FLAG) == 0 );
         Card->ReadData[i] = SD_CARD_EUSCI->RXBUF;

         while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
         SD_CARD_EUSCI->TXBUF = 0xFF;  //Send no data, but trigger clk cycles
     }

     while( (SD_CARD_EUSCI->IFG & SD_CARD_RX_FLAG) == 0);
         Card->ReadCRC[0] = SD_CARD_EUSCI->RXBUF;


     while( (SD_CARD_EUSCI->IFG & SD_CARD_TX_FLAG) == 0);
     SD_CARD_EUSCI->TXBUF = 0xFF;  //Send no data, but trigger clk cycles

     while( (SD_CARD_EUSCI->IFG & SD_CARD_RX_FLAG) == 0);
         Card->ReadCRC[1] = SD_CARD_EUSCI->RXBUF;

    (SD_CARD_PORT->OUT) |= SD_CARD_CS_PIN;
    return true;
}
