#ifndef I2C_H_
#define I2C_H_

#include <stdbool.h>
#include "msp432.h"

#define EUSCI_PORT         P3
#define EUSCI_X            EUSCI_B2
#define I2C_CTL           (EUSCI_X->CTLW0)
#define I2C_STAT          (EUSCI_X->STATW)
#define I2C_RXBUF         (EUSCI_X->RXBUF)
#define I2C_TXBUF         (EUSCI_X->TXBUF)
#define I2C_CLKDIVIDE     (EUSCI_X->BRW)
#define I2C_IE            (EUSCI_X->IE)
#define I2C_IFG           (EUSCI_X->IFG)
#define I2C_IRQn           EUSCIB1_IRQn
#define I2C_IRQHandler     EUSCIB1_IRQHandler

#define WRITE_SIZE            100
#define WRITE_ADDRESS_SIZE    2
#define READ_ADDRESS_SIZE     2
#define READ_SIZE             100
#define CLKSPEED              30 //3000000/30 = 100000 :)


#define ADXL_ADDRESS         0x53
#define MSG_DELIMITER       ','
//#define ADXL_WRITE           0xA6
//#define ADXL_READ            0xA7

#define SAM_M8Q_ADDRESS      0x42
#define SAM_M8Q_AVAIL_BYTES_UPPER_BYTE_REGISTER   0xFD
#define SAM_M8Q_AVAIL_BYTES_LOWER_BYTE_REGISTER   0xFE
#define SAM_M8Q_DATA_STREAM_REGISTER              0xFF

#define GPS_ADDRESS          0x42

typedef struct NMEA_RMC_MSG{
      char MSG_ID[7];   //  ex) $GNRMC, $GPRMC
      char UTC_Time[10]; //  hhMMss.ss
      char Validity;    // A = Valid, V = Not Valid
      char Latitude[11];     // ddMM.M M M M M
      char North_South;
      char Longitude[12];    // dddMM.M M M M M
      char East_West;
      char Speed[6];         // Speed over Ground in Knots
      char COG[6];           //Course over ground in degrees
      char Date[7];         //ddMMyy
      char Magnetic_Variation[6];
      char MV_East_West;
      char Position_Mode[5];
}NMEA_RMC_MSG;


typedef struct I2C
{
    uint8_t dataToWrite[100];
    uint8_t writeAddress;
    uint8_t deviceAddress;
    uint8_t readAddress;
    uint8_t dataToRead[100];
} I2C;

bool initI2C(void);
void purgeI2C(I2C*);
bool writeI2C(I2C, int);
bool readI2C(I2C*, int);

#endif /* I2C_H_ */
