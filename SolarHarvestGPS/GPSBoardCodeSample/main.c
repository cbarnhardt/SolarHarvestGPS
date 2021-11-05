/*
 * Name: Jesse Fernandez, Collin Barnhardt
 * Class: EGR 436 - 10
 * Instructor: Dr. Brakora, Dr. Jiao
 * Date: April 22, 2021
 * Purpose: Log positional data of the module
 * Program Description:
 *          This program interfaces with an accelerometer (ADXL 343), a GPS module (SAM-M8Q), and MicroSD card to log positional
 *      data of the module. The accelerometer is used to detect motion. When motion is detected, the module becomes active, waking the GPS module
 *      from low power mode. Once the SAM-M8Q is ready, it is hailed for positional data. It is expected that the SAM-M8Q module is already configured
 *      such that it only sends NMEA RMC messages through I2C communications. Later revision of this module may include the ability to send configuration
 *      messages to the GPS module to create this setup. Currently, the setup is configured externally via U-Center software provided freely by U-Blox.
 *          After becoming active, the system is kept active for 1 minute. During this time, GPS data is stored in the SD card, which can be viewed
 *      via disk editors, such as HxD. The system also displays the data being saved in real time via USB serial port and Bluetooth Low Energy (BLE).
 *      AFter 1 minute has passed, if no further activity is detected, then the system goes into sleep mode, puting the GPS module into a low power mode.
 *      It then stays in this state until the accelerometer detects motion, which causes the cycle to repeat.
 */

#include "msp.h"
#include "stdio.h"

#include "abstraction.h"
#include "I2C.h"
#include "Timing.h"

#include "ADXL343.h"
#include "SD_Card.h"
#include "Communication.h"
#include "UART.h"

#define EXTINT_PORT_OUT (P2->OUT)
#define EXTINT_PIN      (BIT7)
#define BUFFER_SIZE     600

/****************** Global Variables *****************************************************************/
bool Delay_End_Flag;    // Flag indicating the period between beats has ended and strobing should occur
bool Active_End_Flag;   // Flag indicating end of activity period
/*****************************************************************************************************/

/*************************** IRQ Declarations *******************************/
void TA0_0_IRQHandler(void);
void TA1_0_IRQHandler(void);
void EUSCIA0_IRQHandler(void);
void EUSCIA3_IRQHandler(void);
/****************************************************************************/

/********************** Custom Function Declarations *******************************/
void Start_Delay(int16_t msec);
void Blocking_Delay(int16_t msec, uint16_t cycles);
void Set_Activity_Timer(void);
void ExtintPinSetup(void);
void SetREFOclk(void);
void SaveToSDcard(SDcard*, NMEA_RMC_MSG [6]);
void ReadSDsetup(SDcard* Card);
void UpdateSDsetup(SDcard* Card);
void WipeSD(SDcard* Card);
void FirstTimeSetup(SDcard* Card);
/***********************************************************************************/

NMEA_RMC_MSG SAM_Reading[6];    // Each index of this structure will hold one NMEA RMC message. Once every index has been filled, will be saved to SD card
SDcard GPS_Log;                 // Structure holding data regarding the SD card being used.

I2C ADXL;   // A software structure for the ADXL, used in I2C comms
I2C SAM;    // A software structure for the SAM-M8Q, used in I2C comms

uint8_t Cycle_Count;

unsigned char GPS_Buffer[BUFFER_SIZE];
unsigned char ADXL_Buffer[BUFFER_SIZE];

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	Delay_End_Flag = 0;
	Active_End_Flag = 0;

	uint8_t MessageIndex = 0;   // Variable used to keep track of which RMC message is being filled out
	bool Sleep_Status = 0;   //Variable indicating if ADXL says we are asleep or not
    bool Sleep_ONS = 0; //Makes it so that any actions taken on Sleep are performed only once
    bool Active_ONS = 0; //Makes it so that any actions taken on detection of activity are performed only once
    bool Stop_Ready = 0; //Flag indicating that the end of a GPS message has been reached and can stop reading if needed

    uint16_t i = 0;
	unsigned short j = 0;
	unsigned char temp = ' ';

	initI2C();
	purgeI2C(&ADXL);
	purgeI2C(&SAM);

	ExtintPinSetup();

	TIMING_TIMER_A_SETUP();
	ACTIVITY_TIMER_A_SETUP();

	SetREFOclk();

	PC_2_MSP_COMM_SETUP();
    BLE_2_MSP_COMM_SETUP();

/****************************** START DEVICE INITIALIZATIONS ****************************************/
	//Require some time on startup for SD card to boot up properly, recommended time 1 second
	Blocking_Delay(1000, 1);

	initSDcard();

	FirstTimeSetup(&GPS_Log);
    ReadSDsetup(&GPS_Log);
    UpdateSDsetup(&GPS_Log);

	__enable_interrupt();

	//ADXL needs some time to boot properly, although still seems to mess up on occasion
	    //Likely due to stopping I2C comms in the middle of activity, confusing the device
	Blocking_Delay(1000, 1);

	/*  Setup of this pin occurrs in ExtintPinSetup()   */
    while(P2->IN & BIT0);   //This is to make it possible to "reset" adxl I2C comms. the ADXL gets stuck if these
                                // these comms are interrupted mid-operation. Current prototype has
                                // a wire jumper from P2.0 to GND to bypass this.

    initADXL(&ADXL);
/******************* END DEVICE INTIALIZATIONS *******************************************/

/******************* START FIRST READ OF ADXL TO CLEAR ITS ISR AND START FROM KNOWN STATE ON POWERUP ******************/
    readI2C(&ADXL, 1);

   if(ADXL.dataToRead[0] & BIT3){   //Poll register checking sleep status of ADXL, 1 means active
       if(Sleep_Status == 0)   //If was active before, do this
           Sleep_ONS = 0;
       Sleep_Status = 1;   //Indicate that ADXL is sleeping
   }
   else{
       if(Sleep_Status == 1)   //If was sleeping before, do this
           Active_ONS = 0;
       Sleep_Status = 0;   //Indicate that ADXL is active
   }

   if( (Sleep_Status == 0) && (Active_ONS == 0)){   //If first run after detecting activity, entering active mode
       Active_ONS = 1;
       Sleep_ONS = 0;
       Blocking_Delay(1000, 2);
       EXTINT_PORT_OUT |= EXTINT_PIN;
       Blocking_Delay(100, 1);
       printToapp("\nEntering Active Mode\n");
       Set_Activity_Timer();
   }

   if( (Sleep_Status == 1) && (Sleep_ONS == 0)){   //If first run after detecting sleep, entering sleep mode
       Sleep_ONS = 1;
       Active_ONS = 0;
       Active_End_Flag = 0;
       EXTINT_PORT_OUT &= ~EXTINT_PIN;
       Blocking_Delay(1000, 2);
       ADXL.readAddress = INT_SOURCE;
       readI2C(&ADXL, 1);
       printToapp("Entering Sleep Mode\n");
   }

   /*
    * These values do not change throughout the program, so only perform them once to save energy and time
    */
   SAM.deviceAddress = SAM_M8Q_ADDRESS;
   SAM.readAddress = SAM_M8Q_DATA_STREAM_REGISTER;

	while(1){
	    while(P2->IN & BIT0);   //This allows for stopping I2C comms in a safe position through
	                                // external jumper. This can be replaced in later designs with a PMOSFET on high side of device
	                                // or NMOSFET on low side of device.
        /*
         * AFter first read ADXL read, MSP knows if the accelerometer thinks
         * the module is in motion. If in active state, start reading GPS data
         */
	    if(Sleep_Status == 0){
            i = 0;
            j = 0;

            readI2C(&SAM, 1);
            temp = SAM.dataToRead[0];

            /***************** START MESSAGE PARSING, follows standard NMEA RMC format **************************/
            if(SAM.dataToRead[0] == '$'){    //If hit start of message...
                Stop_Ready = 0;
                do{
                    switch(i){
                        case 0:
                            SAM_Reading[MessageIndex].MSG_ID[j] = temp;
                            break;
                        case 1:
                            SAM_Reading[MessageIndex].UTC_Time[j] = temp;
                            break;
                        case 2:
                            SAM_Reading[MessageIndex].Validity = temp;
                        case 3:
                            SAM_Reading[MessageIndex].Latitude[j] = temp;
                            break;
                        case 4:
                            SAM_Reading[MessageIndex].North_South = temp;
                            break;
                        case 5:
                            SAM_Reading[MessageIndex].Longitude[j] = temp;
                            break;
                        case 6:
                            SAM_Reading[MessageIndex].East_West = temp;
                            break;
                        case 7:
                            SAM_Reading[MessageIndex].Speed[j] = temp;
                            break;
                        case 8:
                            SAM_Reading[MessageIndex].COG[j] = temp;
                            break;
                        case 9:
                            SAM_Reading[MessageIndex].Date[j] = temp;
                            break;
                        case 10:
                            SAM_Reading[MessageIndex].Magnetic_Variation[j] = temp;
                            break;
                        case 11:
                            SAM_Reading[MessageIndex].MV_East_West = temp;
                            break;
                        case 12:
                            SAM_Reading[MessageIndex].Position_Mode[j] = temp;
                            break;
                        default:
                            break;
                    }
                    j++;
                    if(j >= 80)
                        j = 0;

                    if(temp == '*'){    //Asterisk means end of meaningful RMC message
                        Stop_Ready = 1;

                        MessageIndex += 1;
                        if(MessageIndex == MAX_NMEA_RMC_MSG_IN_BLOCK){
                            SaveToSDcard(&GPS_Log, SAM_Reading);
                            MessageIndex = 0;
                        }
                        break;
                    }
                    else
                        Stop_Ready = 0;

                    readI2C(&SAM, 1);
                    temp = SAM.dataToRead[0];

                    if(temp == MSG_DELIMITER){
                        i++;
                        j = 0;
                    }
                }while(Stop_Ready == 0);
            }
            else{
                Stop_Ready = 1;
            }
            /**************************** END MESSAGE PARSING *************************/
        }

        if( (Active_End_Flag == 1) && (Stop_Ready == 1) ){  //If ready to stop activity and go into low power modes
            ADXL.readAddress = INT_SOURCE;  //Must read this register of the ADXL to clear its interrupt
            readI2C(&ADXL, 1);
            Sleep_Status = 1;
        }

        /*************** CHECK ADXL TO SEE IF ACTIVITY HAS BEEN DETECTED **/
        ADXL.readAddress = ACT_TAP_STATUS;
        readI2C(&ADXL, 1);

        if(ADXL.dataToRead[0] & BIT3){   //Poll register holding sleep status of ADXL, logical 0 means active
            if(Sleep_Status == 0)   //If was active before, do this
                Sleep_ONS = 0;
            Sleep_Status = 1;   //Indicate that ADXL is sleeping
        }
        else{
            if(Sleep_Status == 1)   //If was sleeping before, do this
                Active_ONS = 0;
            Sleep_Status = 0;   //Indicate that ADXL is active
        }
        /*************** END CHECK ****************************************/

        if( (Sleep_Status == 0) && (Active_ONS == 0) ){   //If first run after detecting activity, entering active mode
            Active_ONS = 1;
            Sleep_ONS = 0;
            Blocking_Delay(1000, 2);    //The GPS module NEEDS some time after comm reads before switching powre states
            EXTINT_PORT_OUT |= EXTINT_PIN;
            Blocking_Delay(100, 1); //The GPS module NEEDS some time after switching power modes to stabilize activity, cannot change states quickly.
            printToapp("\nEntering Active Mode\n");
            Set_Activity_Timer();
        }

        if( (Sleep_Status == 1) && (Sleep_ONS == 0) ){   //If first run after detecting sleep, entering sleep mode
            Sleep_ONS = 1;
            Active_ONS = 0;
            Active_End_Flag = 0;
            EXTINT_PORT_OUT &= ~EXTINT_PIN;
            Blocking_Delay(1000, 2);    //The GPS module NEEDS some time after swtiching powre modes to stabilize, cannot change states quickly.
            ADXL.readAddress = INT_SOURCE;  //Make sure to read registers of ADXL to make sure it behaves as expected.
            readI2C(&ADXL, 1);
            printToapp("\nEntering Sleep Mode\n");
        }
	}
}

/*
 * Helper function used to start the timer.
 */
void Start_Delay(int16_t msec){
    Delay_End_Flag = 0;
    TIMING_TIMER_A_CTL &= ~TIMER_A_UP_MODE; //Stop timer
    TIMING_TIMER_A_CTL |= TIMER_A_COUNT_CLR;    //Clear count
    TIMING_TIMER_A_CCR_0 = TIMER_A_COUNT_1_MSEC * msec; //Set delay
    TIMING_TIMER_A_CTL |= TIMER_A_UP_MODE;
}

/*
 * This is a blocking delay, stopping flow of code until timer runs out.
 *  The maximum msec in is 1984, Timer A can hold a max of 65535 cycles.
 *  To have longer delays, multiply delay by the input cycles.
 */
void Blocking_Delay(int16_t msec, uint16_t cycles){
    uint16_t j = 0;
    for(j = 0; j < cycles; j++){
        Start_Delay(msec);
        while(Delay_End_Flag == 0);
        Delay_End_Flag = 0;
    }
}

/*
 * Set the 60 second timer for which the module is active.
 */
void Set_Activity_Timer(void){
    Active_End_Flag = 0;
    ACTIVITY_TIMER_A_CTL &= ~TIMER_A_UP_MODE; //Stop timer
    ACTIVITY_TIMER_A_CTL |= TIMER_A_COUNT_CLR;    //Clear count
    ACTIVITY_TIMER_A_CCR_0 = 30720; //Set delay
    ACTIVITY_TIMER_A_CTL |= TIMER_A_UP_MODE;
}

/*
 * Set up the pin controlling the power saving mode of the SAM-M8Q
 * and also setup the pin used to cause the program to hang so that it
 * can be safely restarted if debugging. This is implemented so because
 * the GPS and ADXL use I2C to communicate. If these comms are interrupted
 * mid-operation (as in when loading a new program through debugging),
 * he device being communicated with at the time becomes confused and stops
 * responding to comms. The ADXL is a notable problem child when it comes
 * to this. This can be fixed through the use of PMOSFET on the high side
 * of the devices or an NMOSFET on the low side of the devices that is
 * controlled by the MSP. Idea for next revision.
 */
void ExtintPinSetup(void){
    P2->SEL0 &= ~0xFF;
    P2->SEL1 &= ~0xFF;
    P2->DIR |= BIT7;
    EXTINT_PORT_OUT &= ~BIT7;

    //setup a pin to stall initializing, to be used if ADXL gets stuck if interrupted during I2C comms
    P2->REN |= BIT0;    //Enable pull-up/pull-down resistor on P2.0
    P2->OUT |= BIT0;    //Make it pull-up resistor
}
/*
 * Turn on the REFO clk source and have it set to be used to for ACLK
 * instead of external clock source LFXT
 */
void SetREFOclk(void){
    CS->KEY = CS_KEY;

    CS->CLKEN |= BIT9;
    CS->CTL1 |= BIT9 | BITC;
    CS->CTL1 &= ~(BIT8 | BITA);

    CS->KEY = 0;
}

/*
 * Saves MAX_NMEA_RMC_MSG_IN_BLOCK number of RMC messages in the SD card.
 * Also prints the data being saved to the serial and BLE UART ports.
 * Once those processes are done, updates the setup block in the SD card
 * to match that of the MSP.
 */
void SaveToSDcard(SDcard* Card, NMEA_RMC_MSG RMCMessage[6]){
    uint16_t i = 0;
    uint8_t MessageIndex = 0;
    uint8_t j = 0;

    for(i = 0; i < SD_BLOCK_LENGTH; i++){
            Card->DataToWrite[i] = *( RMCMessage[MessageIndex].MSG_ID + j);  //RMCMessage variables are saved in memory sequentially
            j++;
            if(Card->DataToWrite[i] == '*'){
                j = 0;
                MessageIndex += 1;
                //printToapp("\n");
            }
            if(MessageIndex == MAX_NMEA_RMC_MSG_IN_BLOCK)
                break;
    }

    /*  Save the GPS Data in sd card memory */
    Card->WriteAddress = Card->NextAvailWriteLocation;
    writeToSDcard(Card);
    Card->NextAvailWriteLocation += 0x00000001;

    printToapp2(Card);
    printToapp("\n");

    /*  Update the saved SD card setup  */
    UpdateSDsetup(Card);
}

/*
 * Read the settings block in the SD card and copy it to the MSP settings holder
 */
void ReadSDsetup(SDcard* Card){
    uint16_t i = 0;

    Card->ReadAddress = SD_SETTINGS_ADDRESS;
    readSDcard(Card);

    for(i = 0; i < SD_BLOCK_LENGTH; i++)
        Card->Setup[i] = Card->ReadData[i];

    /*  Get the next available write location that's saved in SD card memory    */
    Card->NextAvailWriteLocation = 0x00000000;
    Card->NextAvailWriteLocation = Card->NextAvailWriteLocation | (Card->Setup[3] << 24);
    Card->NextAvailWriteLocation = Card->NextAvailWriteLocation | (Card->Setup[2] << 16);
    Card->NextAvailWriteLocation = Card->NextAvailWriteLocation | (Card->Setup[1] << 8);
    Card->NextAvailWriteLocation = Card->NextAvailWriteLocation | Card->Setup[0];
}

/*
 * Update the settings block save in the SD card to ensure
 * the copy on the MSP matches that of the SD card
 */
void UpdateSDsetup(SDcard* Card){
    uint16_t i = 0;

    /*  Hard limit of the SD card addressing range discovered experimentally    */
    if( (Card->NextAvailWriteLocation > MAX_DATA_ADDRESS) || (Card->NextAvailWriteLocation < SD_DATA_START_ADDRESS) )
        Card->NextAvailWriteLocation = SD_DATA_START_ADDRESS;

    Card->Setup[0] = Card->NextAvailWriteLocation & 0x000000FF;
    Card->Setup[1] = (Card->NextAvailWriteLocation & 0x0000FF00) >> 8;
    Card->Setup[2] = (Card->NextAvailWriteLocation & 0x00FF0000) >> 16;
    Card->Setup[3] = (Card->NextAvailWriteLocation & 0xFF000000) >> 24;

    for(i = 0; i < SD_BLOCK_LENGTH; i++)
        Card->DataToWrite[i] = Card->Setup[i];

    Card->WriteAddress = SD_SETTINGS_ADDRESS;
    writeToSDcard(Card);

    ReadSDsetup(Card);
}

/*  Completely wipes the data of the SD card by writing DUMMY_DATA to the addresses */
void WipeSD(SDcard* Card){
    uint32_t i = 0;
    Card->NextAvailWriteLocation = SD_DATA_START_ADDRESS;
    Card->WriteAddress = SD_SETTINGS_ADDRESS;

    for(i = 0; i < SD_BLOCK_LENGTH; i++)
        Card->DataToWrite[i] = DUMMY_DATA;

    for(i = 0; i <= MAX_DATA_ADDRESS; i++){
        writeToSDcard(Card);
        Card->WriteAddress += 0x00000001;
    }

    UpdateSDsetup(Card);    //Setup the Settings address in the SD card
}

/*
 * If using a new SD card for data storage, need to setup the "settings"
 * part of the SD card for use in this design. SImply writes to the
 * SD_SETTINGS_ADDRESS macro found in SD_Card.h. This is done so that if MSP
 * loses power, it does not have to write over old data by starting at the
 * very beginning of the Data address range
 */
void FirstTimeSetup(SDcard* Card){
    uint16_t i = 0;
    bool Rewrite = 1;

    Card->ReadAddress = SD_SETTINGS_ADDRESS;
    readSDcard(Card);

    for(i = 0; i < SD_BLOCK_LENGTH; i++)
        if(Card->ReadData[i] != DUMMY_DATA)
            Rewrite = 0;
    if(Rewrite){
        for(i = 0; i < SD_BLOCK_LENGTH; i++)
            Card->DataToWrite[i] = 0x00;

        Card->WriteAddress = SD_SETTINGS_ADDRESS;
        Card->DataToWrite[0] = (SD_DATA_START_ADDRESS & 0x000000FF);
        Card->DataToWrite[1] = (SD_DATA_START_ADDRESS & 0x0000FF00) >> 8;
        Card->DataToWrite[2] = (SD_DATA_START_ADDRESS & 0x00FF0000) >> 16;
        Card->DataToWrite[3] = (SD_DATA_START_ADDRESS & 0xFF000000) >> 24;
        writeToSDcard(Card);
    }
}
/************************ INTERRUPT HANDLERS **********************/
void EUSCIA0_IRQHandler(void){
    if( (PC_UART_IFG & PC_UART_RX_IRQ_FLAG) && (PC_UART_IE & PC_UART_IE) ){
        PC_UART_IFG &= ~PC_UART_RX_IRQ_FLAG;
    }
}

void EUSCIA3_IRQHandler(void){
    if( (BLE_UART_IFG & BLE_UART_RX_IRQ_FLAG) && (BLE_UART_IE & BLE_UART_IE) ){
        BLE_UART_IFG &= ~BLE_UART_RX_IRQ_FLAG;
    }
}

void TA0_0_IRQHandler(void){
    if( (ACTIVITY_TIMER_A_CCTL_0 & TIMER_A_CCTL_IFG) && (ACTIVITY_TIMER_A_CCTL_0 & TIMER_A_CCTL_IRQ_EN) ){
        ACTIVITY_TIMER_A_CCTL_0 &= ~TIMER_A_CCTL_IFG;
        ACTIVITY_TIMER_A_CTL &= ~TIMER_A_UP_MODE;
        Active_End_Flag = 1;
    }
}

void TA1_0_IRQHandler(void){
    if( (TIMING_TIMER_A_CCTL_0 & TIMER_A_CCTL_IFG) && (TIMING_TIMER_A_CCTL_0 & TIMER_A_CCTL_IRQ_EN) ){
        TIMING_TIMER_A_CCTL_0 &= ~TIMER_A_CCTL_IFG;
        TIMING_TIMER_A_CTL &= ~TIMER_A_UP_MODE;
        Delay_End_Flag = 1;
    }
}
