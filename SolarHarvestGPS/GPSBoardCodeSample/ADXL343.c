#include "ADXL343.h"
#include <stdbool.h>
#include "msp432.h"
#include "I2C.h"

/*
 * Configure the registers in the ADXL such that the ADXL behaves
 * as desired for this application.
 */
bool initADXL(I2C* device)
{
    (*device).deviceAddress = ADXL_ADDRESS;

    (*device).readAddress = INT_SOURCE;
    readI2C(device, 1);

    (*device).dataToWrite[0] = 0;  //Put part into standby mode
        (*device).writeAddress = POWER_CTL;
        /*
        * 0x2D is the POWER_CTL register:
        *  D7: ALWAYS ZERO
        *  D6: ALWAYS ZERO
        *  D5: Link
        *      - If 1 and both activity and inactivity functions are enabled, makes delay on start of the activity function until inactivity is detected
        *      - If 0, activity and inactivity functions are concurrent.
        *  D4: AUTO_SLEEP
        *      - If link bit is set, setting this to 1 enables auto-sleep functionality
        *  D3: Measure
        *      - Setting to 0 puts part into standby mode. Setting to 1 places in measurement mode.
        *          *Note: On powerup, ADXL343 is in standby mode (set to 0)
        *  D2: Sleep
        *      - Setting to 0 puts part into normal operation. Setting to 1 puts into sleep mode.
        *          Sleep mode suppresses DATA_READY, stops transmission of data to FIFO, switches sampling
        *              rate to one specified by wakeup bits.
        *  D1 -> D0; Wakeup
        *      - Controls frequency of reading in sleep mode according to Table 20 in documentation
        */
        writeI2C((*device), 1); // write mods into register


   //set Activity threshold, 62.5mg/LSB scale factor
   //in this case set to 0.125g
   (*device).writeAddress = THRESH_ACT;
   (*device).dataToWrite[0] = BUMP_LIM;
   /*
    * 0x24 is the THRESH_ACT register, which holds the activity threshold value for detecting activity:
    *       The register is 8 bits. Data format is unsigned. Scale factor is 62.5 mg/LSB.
    *
    *       Threshold = dataToWrite[0] * 62.5 mg/LSB = 125 mg/LSB
    */
   writeI2C((*device), 1);


   //set time inactivity register, 1 sec/LSB
   //in this case set to 60 seconds
   (*device).writeAddress = TIME_INACT;
   (*device).dataToWrite[0] = 0; //0x3C is 60 in decimal
   /*
    * 0x26 is the TIME_INACT register, which holds the amount of time that acceleration must be less than
    *   to be declared as inactive.
    *       The register is 8 bits. Data format is unsigned. Scale factor is 1 sec/LSB.
    *
    *       Inactivity Time  = dataToWrite[0] * 1 sec/LSB = 60 sec/LSB
    */
   writeI2C((*device), 1);

   //set inactivity threshold, 62.5mg/LSB
   //in this case set to 0.125g
   (*device).writeAddress = THRESH_INACT;
   (*device).dataToWrite[0] = BUMP_LIM;
    /*
    * 0x25 is the THRESH_INACT register, which holds the activity threshold value for detecting inactivity:
    *       The register is 8 bits. Data format is unsigned. Scale factor is 62.5 mg/LSB.
    *
    *       Threshold = dataToWrite[0] * 62.5 mg/LSB = 125 mg/LSB
    */
   writeI2C((*device), 1);


   //Enable act inact detection DC Mode 3-axis participation
   (*device).writeAddress =  ACT_INACT_CTL;
   (*device).dataToWrite[0] = (BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
    /*
    * 0x27 is the ACT_INACT_CTL register:
    *  D7: ACT ac/dc
    *       - If 1, sets for ac-coupled operation for activity detection. See datasheet for operation description, page 22 of 36
    *       - If 0, sets for dc-coupled operation for activity detection. See datasheet for operation description, page 22 of 36
    *  D6: ACT_X enable
    *       - If 1, enables X-axis acceleration to participate in detecting activity
    *       - If 0, excludes X-axis acceleration from participating in detection of activity
    *  D5: ACT_Y enable
    *       - If 1, enables Y-axis acceleration to participate in detecting activity
    *       - If 0, excludes Y-axis acceleration from participating in detection of activity
    *  D4: ACT_Z enable
    *       - If 1, enables Z-axis acceleration to participate in detecting activity
    *       - If 0, excludes Z-axis acceleration from participating in detection of activity
    *  D3: INACT ac/dc
    *       - If 1, sets for ac-coupled operation for activity detection. See datasheet for operation description, page 22 of 36
    *       - If 0, sets for dc-coupled operation for activity detection. See datasheet for operation description, page 22 of 36
    *  D2: INACT_X enable
    *       - If 1, enables X-axis acceleration to participate in detecting inactivity
    *       - If 0, excludes X-axis acceleration from participating in detection of inactivity
    *  D1: INACT_Y enable
    *       - If 1, enables Y-axis acceleration to participate in detecting inactivity
    *       - If 0, excludes Y-axis acceleration from participating in detection of inactivity
    *  D0: INACT_O enable
    *       - If 1, enables Z-axis acceleration to participate in detecting inactivity
    *       - If 0, excludes Z-axis acceleration from participating in detection of inactivity
    */
   writeI2C((*device), 1);



   //map interrupts to INT1 or INT2
   //sets Activity and Inactivity to INT2 Pin
   //MUST READ THE INT_SOURCE REG TO CLEAR THESE INTERRUPTS
   (*device).writeAddress = INT_MAP;
   (*device).dataToWrite[0] = (BIT4);
   //(*device).dataToWrite[0] = (BIT4|BIT3);
   /*
   * 0x2F is the INT_MAP register.
   *    Any bit set to zero sends the corresponding functions interrupt to INT1 pin.
   *    Any bit set to one sends the corresponding functions interrupt to INT2 pin:
   *  D7: DATA_READY
   *  D6: SINGLE_TAP
   *  D5: DOUBLE_TAP
   *  D4: ACTIVITY
   *  D3: INACTIVITY
   *  D2: FREE_FALL
   *  D1: WATERMARK
   *  D0: OVERRUN
   */
   writeI2C((*device), 1);


   //enable act inact interrupts
   (*device).writeAddress = INT_ENABLE;
   (*device).dataToWrite[0] = (BIT4|BIT3);
   //(*device).dataToWrite[0] = (BIT4|BIT3);
    /*
    * 0x2E is the INT_ENABLE register.
    *    Any bit set to zero disables the corresponding interrupt.
    *    Any bit set to one enables the corresponding interrupt:
    *  D7: DATA_READY
    *  D6: SINGLE_TAP
    *  D5: DOUBLE_TAP
    *  D4: ACTIVITY
    *  D3: INACTIVITY
    *  D2: FREE_FALL
    *  D1: WATERMARK
    *  D0: OVERRUN
    */
   writeI2C((*device), 1);

   (*device).readAddress = INT_SOURCE;
    /*
    * 0x30 is the INT_SOURCE register.
    *    Any bit set to zero indicates the corresponding interrupt has not triggered.
    *    Any bit set to one indicates the corresponding interrupt has triggered:
    *    Reading this register clears the interrupts of
    *       - SINGLE_TAP
    *       - DOUBLE_TAP
    *       - ACTIVITY
    *       - INACTIVITY
    *       - FREE_FALL
    *  D7: DATA_READY
    *  D6: SINGLE_TAP
    *  D5: DOUBLE_TAP
    *  D4: ACTIVITY
    *  D3: INACTIVITY
    *  D2: FREE_FALL
    *  D1: WATERMARK
    *  D0: OVERRUN
    */
   readI2C(device, 1);
   //(*device).dataToWrite[0] = (BIT4 | BIT3);  //Take part out of standby mode and into measurement mode
    (*device).dataToWrite[0] = (BIT5 | BIT4 | BIT3);  //Take part out of standby mode and into measurement mode
    //(*device).dataToWrite[0] = (BIT3);  //Take part out of standby mode and into measurement mode
    (*device).writeAddress = POWER_CTL;
    /*
    * 0x2D is the POWER_CTL register:
    *  D7: ALWAYS ZERO
    *  D6: ALWAYS ZERO
    *  D5: Link
    *      - If 1 and both activity and inactivity functions are enabled, makes delay on start of the activity function until inactivity has first been detected.
    *           Then, cycles between
    *      - If 0, activity and inactivity functions are concurrent.
    *  D4: AUTO_SLEEP
    *      - If link bit is set, setting this to 1 enables auto-sleep functionality
    *  D3: Measure
    *      - Setting to 0 puts part into standby mode. Setting to 1 places in measurement mode.
    *          *Note: On powerup, ADXL343 is in standby mode (set to 0)
    *  D2: Sleep
    *      - Setting to 0 puts part into normal operation. Setting to 1 puts into sleep mode.
    *          Sleep mode suppresses DATA_READY, stops transmission of data to FIFO, switches sampling
    *              rate to one specified by wakeup bits.
    *  D1 -> D0; Wakeup
    *      - Controls frequency of reading in sleep mode according to Table 20 in documentation
    */
    writeI2C((*device), 1); // write mods into register

   return true;
}
