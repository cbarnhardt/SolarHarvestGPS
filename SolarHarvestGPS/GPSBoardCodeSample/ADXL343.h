#ifndef ADXL343_H_
#define ADXL343_H_

#include "I2C.h"


#define ADXL_ADDRESS    0x53

      //REG NAME    REG ADDRESS     TYPE    Desc 
#define ADXL343_ID      0x00        //R     Device ID
#define THRESH_TAP      0x1D        //RW    Tap Threshold
#define OFSX            0x1E        //RW    X-Axis Offset
#define OFSY            0x1F        //RW    Y-Axis Offset
#define OFSZ            0x20        //RW    Z-Axis Offset
#define DUR             0x21        //RW    Tap Duration
#define LATENT          0x22        //RW    Tap Latency
#define WINDOW          0x23        //RW    Tap Window
#define THRESH_ACT      0x24        //RW    Activity Threshold
#define THRESH_INACT    0x25        //RW    Inactivity Threshold
#define TIME_INACT      0x26        //RW    Inactivity Time
#define ACT_INACT_CTL   0x27        //RW    Axis Enable Control inactivity/activity detection
#define THRESH_FF       0x28        //RW    Free-Fall Threshold
#define TIME_FF         0x29        //RW    Free-Fall Time 
#define TAP_AXES        0x2A        //RW    Axis Control for single tap/double tap
#define ACT_TAP_STATUS  0x2B        //R     Source of Single Tap/Double tap
#define BW_RATE         0x2C        //RW    Data rate and power mode control 
#define POWER_CTL       0x2D        //RW    Power saving features control
#define INT_ENABLE      0x2E        //RW    Interrupt enable control
#define INT_MAP         0x2F        //RW    Interrupt mapping control    
#define INT_SOURCE      0x30        //R     Source of interrupts
#define DATA_FORMAT     0x31        //RW    Data format control
#define DATAX0          0x32        //R     X-Axis data 0
#define DATAX1          0x33        //R     X-Axis data 1
#define DATAY0          0x34        //R     Y-Axis data 0
#define DATAY1          0x35        //R     Y-Axis data 1
#define DATAZ0          0x36        //R     Z-Axis data 0
#define DATAZ1          0x37        //R     Z-Axis data 1
#define FIFO_CTL        0x38        //RW    FIFO control 
#define FIFO_STATUS     0x39        //R     FIFO Status 


#define BUMP_LIM    6               //Do not set less than 2, messes with things for some reason other than making it very sensitive

bool initADXL(I2C*);

#endif /* ADXL343_H_ */
