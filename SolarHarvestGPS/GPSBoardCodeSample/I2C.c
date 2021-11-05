#include "I2C.h"
#include <stdbool.h>

/*
 * Setup a EUSCI port for use in I2C for communicating with ADXL and SAM-M8Q
 */
bool initI2C(void)
{
    I2C_CTL = BIT0;
    I2C_CLKDIVIDE = CLKSPEED;
    I2C_CTL = 0x0F81; //Single master, SMCLK Source, 7-bit address 0000 1111 1000 0001
    EUSCI_PORT->SEL0 |= (BIT7|BIT6); //set up port 3 pins for I2C
    EUSCI_PORT->SEL1 &=~(BIT7|BIT6);
    I2C_CTL &= ~BIT0;

    return true;
}

/*
 * Make sure that any I2C structure that is instantiated is cleared and
 * starts from known state
 */
void purgeI2C(I2C* buff)
{
    int i=0;
    for(i = 0; i < WRITE_SIZE; i++)
    {
        (*buff).dataToWrite[i] = 0;
    }
    i=0;
    for (i = 0; i < READ_SIZE; i++)
    {
        (*buff).dataToRead[i] = 0;
    }

    (*buff).writeAddress = 0;
    (*buff).readAddress = 0;


}

/*
 * Write any desired number of bytes through I2C to device. Follows standard
 * I2C protocol.
 */
bool writeI2C(I2C buff, int numBytes)
{
    __disable_interrupt();

    // 0x53 is ADXL_ADDRESS
    // 0x42 is SAM_M8Q_ADDRESS
    EUSCI_X -> I2CSA = buff.deviceAddress;

    EUSCI_X-> CTLW0 |= 0x0010;  //Configure MSP to be Transmitter
    EUSCI_X-> CTLW0 |= 0x0002;  //Generate START condition
    while( (EUSCI_X->CTLW0 & BIT1) );   //Wait for START to clear, which happens after generating START and sending Slave Address

    EUSCI_X->TXBUF = buff.writeAddress;
    while( !(EUSCI_X->IFG & BIT1) );    //Wait for whatever is in TXBUF to be shifted out onto data line


    int i = 0;
    for(i = 0; i < numBytes; i++)
    {
        EUSCI_X->TXBUF = buff.dataToWrite[i];
        while( !(EUSCI_X->IFG & BIT1) );    //Wait for whatever is in TXBUF to be shifted out onto data line
    }

    EUSCI_X->CTLW0 |= 0x0004;   //Generate STOP
    while(EUSCI_X->CTLW0 & 4);  //Wait for STOP condition to clear

    __enable_interrupt();

    return true;

}

/*
 * Read any desired number of bytes through I2C from device. Follows standard
 * I2C protocol.
 */
bool readI2C(I2C* buff, int numBytes)
{
    __disable_interrupt();

    // 0x53 is ADXL_ADDRESS
    // 0x42 is SAM_M8Q_ADDRESS
    EUSCI_X->I2CSA = (*buff).deviceAddress;

    EUSCI_X->CTLW0 |= BIT4; //Configure MSP to be Transmitter
    EUSCI_X->CTLW0 |= BIT1;   //Generate a START condition, which sends START + Slave Address
    while( (EUSCI_X->CTLW0 & BIT1) );   //Wait for START to clear, which happens after generating START and sending Slave Address

    EUSCI_X->TXBUF = (*buff).readAddress;
    while( !(EUSCI_X->IFG & BIT1) );    //Wait for whatever is in TXBUF to be shifted out onto data line

    EUSCI_X->CTLW0 &= ~BIT4;  //Configure MSP to be Receiver
    EUSCI_X->CTLW0 |=  BIT1;  //Generate a start condition

    while( (EUSCI_X->CTLW0 & BIT1) );   //Wait for START to clear, which happens after generating START and sending Slave Address

    int i = 0;
    for(i = 0; i < numBytes; i++)
    {

        if((numBytes - i) == 1)
        {
            EUSCI_X -> CTLW0 |= BIT2;   //If this is the last byte we want to read, generate a stop after
        }

        while(!(EUSCI_X->IFG & BIT0)); //Wait for the RX Flag to go high, indicating byte received
        (*buff).dataToRead[i] = EUSCI_X->RXBUF; //Save the received byte; Reading the buffer also clears the RX Flag
    }

    while(EUSCI_X->CTLW0 & BIT2);   //Wait for the STOP condition to clear

    __enable_interrupt();

    return true;

}

