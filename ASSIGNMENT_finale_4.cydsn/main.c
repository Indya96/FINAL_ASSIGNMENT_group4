/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
/******************************************************************************************************************/
/*                                                   INCLUSIONS                                                   */
/******************************************************************************************************************/

#include "project.h"
#include "InterruptRoutines.h"
#include "I2C_Interface.h"
#include "SPI_Interface.h"
#include "stdio.h"
#include "Pages.h"
#include "25LC256.h"
#include "Registers.h"


/******************************************************************************************************************/
/***************************************************** MAIN *******************************************************/
/******************************************************************************************************************/

int main(void)
{
    /******************************************************************************************************************/
    /*                                            VARIABLES DECLARATION                                               */
    /******************************************************************************************************************/

    //Degug
    char message[50];

    //Acceleration variables
    int16   x;
    uint8_t xl;
    uint8_t xh;
    int16   y;
    uint8_t yl;
    uint8_t yh;
    int16   z;
    uint8_t zl;
    uint8_t zh;
    int16 x_uart;
    int16 y_uart;
    int16 z_uart;

    //Old setup
    uint8_t SETUP_CORRUPTED = 0;
    uint8_t OLD_FSR;
    uint8_t OLD_SF;
    uint8_t OLD_AS;
    uint8_t OLD_BS;

    //Flags setup
    SETUP_FSR_CHANGED = 0;
    SETUP_SF_CHANGED  = 0;
    SETUP_AS_CHANGED  = 0;
    SETUP_BS_CHANGED  = 0;
    SETUP_FSR         = 0;
    SETUP_SF          = 0;

    //Flags INFO
    FLAG_FSR = 0;
    FLAG_SF  = 0;
    FLAG_AS  = 0;
    FLAG_BS  = 0;

    //Flags options
    OPTION_FSR = 0;
    OPTION_SF  = 0;
    OPTION_AS  = 0;



    //Data Variables
    uint8_t   Out_Data[DATA_PACKET_DIM];

    //Number of bytes to be written during page writing and pointer to data to be written.
    uint8_t Bytes_for_page= 64;
    uint8_t * data_pointer;


    //Flags Flow
    StartAcquisition_flag = 0;
    StartStream_flag      = 0;

    //Flags notifications
    summary_ready     = 0;
    push_button_event = 0;
    full_or_empty     = 0;

    //Comunication
    uint8_t  Packets[192];
             SensorDataReady  = 0;
    uint8_t  index            = 0;
    uint8_t  i            = 0;
    uint16_t Adress_to_read;
    uint16_t Adress_to_write;
    uint8_t  UART_OutArray[UART_PACKET_DIM];
    UART_OutArray[0]                   = 0xA0; //header
    UART_OutArray[UART_PACKET_DIM - 1] = 0xC0; //tail
    ErrorCode error;

    /******************************************************************************************************************/
    /*                                               COMPONENTS START                                                 */
    /******************************************************************************************************************/

    UART_Start();
    SPIM_1_Start();
    I2C_Peripheral_Start();
    CyDelay(10);
    AMux_Start();
    ADC_Start();
    Timer_sensor_Start();

    /******************************************************************************************************************/
    /*                                              INTERRUPTs ENABLE                                                 */
    /******************************************************************************************************************/

    CyGlobalIntEnable;                      /* Enable global interrupts. */
    isr_RX_StartEx(Custom_ISR_RX);          /* UART on RX */
    ISR_PB_LOW_StartEx(ISR_PB_LOW);         /* Push button LOW */
    ISR_PB_HIGH_StartEx(ISR_PB_HIGH);       /* Push button HIGH */
    isr_FIFO_StartEx(Custom_FIFO_ISR);      /* FIFO interrupt enabled pointing to proper function adress. */
    isr_TIMER_StartEx(Custom_TIMER_ISR);    /* TIMER interrupt enabled pointing to proper function adress. */
    pin_LED_EXT_Write(0);                   /* turning off the external led */

    /******************************************************************************************************************/
    /*                                  RESETTING EEPROM 0x0000 and 0x0001 INDEX                                      */
    /******************************************************************************************************************/
     /*I Check the adress where i want to start writing*/
    uint8_t EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
    uint8_t EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
    uint16_t EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);

    if (EEPROM_Adress<EEPROM_INITIAL_ADDRESS)
    {
        // Resetting pointer's value on EEPROM
        EEPROM_writeByte(0x0000, (uint8_t) EEPROM_INITIAL_ADDRESS);
        EEPROM_waitForWriteComplete();
        EEPROM_writeByte(0x0001, (uint8_t) 0);
        EEPROM_waitForWriteComplete();
    }

    // Creating a new pointer
    EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
    EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
    EEPROM_Adress = (uint16_t) (EEPROM_Adress_Pointer_H<<8) | EEPROM_Adress_Pointer_L ;

    UART_PutString("\r\n");
    sprintf(message,"     EEPROM will be written starting from address: 0x%x\r\n", EEPROM_Adress);
    UART_PutString(message);

    /******************************************************************************************************************/
    /*                                            RETRIVING OLD SETUP                                                 */
    /******************************************************************************************************************/

    UART_PutString("\r\n");
    UART_PutString("     Retriving old setup...\r\n");
    UART_PutString("\r\n");

    /* reading from EEPROM old Sampling Frequency */
    OLD_SF = EEPROM_readByte(EEPROM_OLD_SF_ADDRESS);
    if (OLD_SF!=0x17 && OLD_SF!=0x27 && OLD_SF!=0x37 && OLD_SF!=0x47){
        UART_PutString("     Old Sampling frequency            CORRUPTED\r\n");
        SETUP_CORRUPTED++;}
    else{
        sprintf(message, "     Old additional sensor      0x%02X RETRIVED\r\n", OLD_SF);
        UART_PutString(message);}

    /* reading from EEPROM old Full Scale Range */
    OLD_FSR = EEPROM_readByte(EEPROM_OLD_FSR_ADDRESS);
    if (OLD_FSR!=0x80 && OLD_FSR!=0x90 && OLD_FSR!=0xA0 && OLD_FSR!=0xB0){
        UART_PutString("     Old full scale range              CORRUPTED\r\n");
        SETUP_CORRUPTED++;}
    else{
        sprintf(message, "     Old full scale range       0x%02X RETRIVED\r\n", OLD_FSR);
        UART_PutString(message);}

    /* reading from EEPROM old Additional sensor */
    OLD_AS = EEPROM_readByte(EEPROM_OLD_AS_ADDRESS);
    if (OLD_AS!=0x00 && OLD_AS!=0x01){
        UART_PutString("     Old additional sensor             CORRUPTED\r\n");
        SETUP_CORRUPTED++;}
    else{
        sprintf(message, "     Old additional sensor      0x%02X RETRIVED\r\n", OLD_AS);
        UART_PutString(message);}

    /* reading from EEPROM old Start/Stop Condition */
    OLD_BS = EEPROM_readByte(EEPROM_OLD_BS_ADDRESS);
    if (OLD_BS!=0x00 && OLD_BS!=0x01){
        UART_PutString("     Old start/stop condition          CORRUPTED\r\n");
        SETUP_CORRUPTED++;}
    else{
        sprintf(message, "     Old start/stop condition   0x%02X RETRIVED\r\n", OLD_BS);
        UART_PutString(message);}

    /******************************************************************************************************************/
    /*                                        CHECKING OLD SETUP INTEGRITY                                            */
    /******************************************************************************************************************/
    /*  IF OLD SETUP ARE CORRUPTED, DEFAULT MODE  */
    /**********************************************/

    if(SETUP_CORRUPTED>0){

        /* Default Start/Stop Condition ***********************************************************************/
        StartAcquisition_flag = 0;
        Timer_sensor_Stop();
        Timer_sensor_ReadStatusRegister();
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG3,
                                             LIS3DH_CTRL_REG3_INT_DISABLED);
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_FIFO_CTRL_REG,
                                             LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG5,
                                             LIS3DH_CTRL_REG5_FIFO_DISABLE);

        UART_PutString("     DEFAULT Start/Stop cond.   : STOP \r\n");

        /* default Sampling Frequency *************************************************************************/
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG1,
                                             LIS3DH_CTRL_REG1_SF_1HZ);
        if (error == NO_ERROR){
            UART_PutString("     DEFAULT sampling frequency : 1Hz \r\n");
            CHANGE_ADC_SF(1000);}
        else{UART_PutString("     Error occurred setting the default sampling frequency \r\n");}

        /* default Full Scale Range and sensitivity ***********************************************************/
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG4,
                                             LIS3DH_CTRL_REG4_FSR_2G);
        if (error == NO_ERROR){
            Sensitivity = 4;
            UART_PutString("     DEFAULT full scale range   : +-2g \r\n");}
        else{UART_PutString("     Error occurred setting the default full scale range \r\n");}

        /* default Additional Sensor **************************************************************************/
        AMux_Select(0);
        UART_PutString("     DEFAULT additional sensor  : Potentiometer \r\n");

        /* saving latter setup on EEPROM **********************************************************************/
        EEPROM_writeByte(EEPROM_OLD_SF_ADDRESS, LIS3DH_CTRL_REG1_SF_1HZ);
        EEPROM_waitForWriteComplete();
        UART_PutString("     Sampling frequency                SUCCESSFULLY SAVED \r\n");
        EEPROM_writeByte(EEPROM_OLD_FSR_ADDRESS, LIS3DH_CTRL_REG4_FSR_2G);
        EEPROM_waitForWriteComplete();
        UART_PutString("     Full scale range                  SUCCESSFULLY SAVED \r\n");
        EEPROM_writeByte(EEPROM_OLD_AS_ADDRESS, 0x01);
        EEPROM_waitForWriteComplete();
        UART_PutString("     Additional sensor                 SUCCESSFULLY SAVED \r\n");
        EEPROM_writeByte(EEPROM_OLD_BS_ADDRESS, 0x00);
        EEPROM_waitForWriteComplete();
        UART_PutString("     Start/stop condition              SUCCESSFULLY SAVED \r\n");
    }

    /**************************************************/
    /*    IF OLD SETUP ARE NOT CORRUPTED, OLD MODE    */
    /**************************************************/

    if(SETUP_CORRUPTED==0){

        /* Changing ADC sampling frequency *******************************************************************/
        switch(OLD_SF){
            case LIS3DH_CTRL_REG1_SF_1HZ:  CHANGE_ADC_SF(1000); break;
            case LIS3DH_CTRL_REG1_SF_10HZ: CHANGE_ADC_SF(100);  break;
            case LIS3DH_CTRL_REG1_SF_25HZ: CHANGE_ADC_SF(40);   break;
            case LIS3DH_CTRL_REG1_SF_50HZ: CHANGE_ADC_SF(20);   break;}

        /* Changing Sensitivity  *****************************************************************************/
        switch(OLD_FSR){
            case LIS3DH_CTRL_REG4_FSR_2G: Sensitivity   = 4;     break;
            case LIS3DH_CTRL_REG4_FSR_4G: Sensitivity   = 8;     break;
            case LIS3DH_CTRL_REG4_FSR_8G: Sensitivity   = 16;    break;
            case LIS3DH_CTRL_REG4_FSR_16G: Sensitivity  = 48;    break;}

        /* Writing LIS3DH Registers **************************************************************************/
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG1,
                                             OLD_SF);
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG4,
                                             OLD_FSR);

        /* Empting FIFO buffer *******************************************************************************/
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_FIFO_CTRL_REG,
                                             LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_FIFO_CTRL_REG,
                                             LIS3DH_FIFO_CTRL_REG_FIFO_MODE);

        /* Switching the right sensor ************************************************************************/
        if(OLD_AS==0){AMux_Select(1);} // LDR
        if(OLD_AS==1){AMux_Select(0);} // POT

        /* Starting or stopping the Acquisition **************************************************************/
        if(OLD_BS==0){
            FLAG_BS = 0;
            StartAcquisition_flag = 0;
            Timer_sensor_Stop();
            Timer_sensor_ReadStatusRegister();
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG3,
                                                 LIS3DH_CTRL_REG3_INT_DISABLED);
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_FIFO_CTRL_REG,
                                                 LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG5,
                                                 LIS3DH_CTRL_REG5_FIFO_DISABLE);

        }
        if(OLD_BS==1){
            StartAcquisition_flag = 1;
            FLAG_BS = 1;
            Timer_sensor_ReadStatusRegister();
            Timer_sensor_Start();
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG3,
                                                 LIS3DH_CTRL_REG3_INT_ENABLE);
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_FIFO_CTRL_REG,
                                                 LIS3DH_FIFO_CTRL_REG_FIFO_MODE);
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG5,
                                                 LIS3DH_CTRL_REG5_FIFO_ENABLE);
            PWM_Start();
            LED_ACQUISITION_ON_TOGGLE();
        }

    }

    /******************************************************************************************************************/
    /*                                            FIFO MODE INITIALIZATION                                            */
    /******************************************************************************************************************/

    UART_PutString("\r\n");
    UART_PutString("     Initializing FIFO mode...\r\n");
    UART_PutString("\r\n");

    error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                         LIS3DH_CTRL_REG3,
                                         LIS3DH_CTRL_REG3_INT_ENABLE);
    if (error == NO_ERROR){UART_PutString("     Overrun interrupt                 ENABLED\r\n");}

    error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                         LIS3DH_FIFO_CTRL_REG,
                                         LIS3DH_FIFO_CTRL_REG_FIFO_MODE);
    if (error == NO_ERROR){UART_PutString("     FIFO mode                         SELECTED\r\n");}

    error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                         LIS3DH_CTRL_REG5,
                                         LIS3DH_CTRL_REG5_FIFO_ENABLE);
    if (error == NO_ERROR){UART_PutString("     FIFO mode                         ENABLED\r\n");}



    /******************************************************************************************************************/
    /*                                                 WELCOME PAGE                                                   */
    /******************************************************************************************************************/

                                                        WELCOME();

    /******************************************************************************************************************/
    /*                                                INFINITE CYCLE                                                  */
    /******************************************************************************************************************/

    for(;;)
    {

        /*******************************************************************************************************************/
        /*                                           HAS BS SETUP BEEN CHANGED?                                            */
        /*******************************************************************************************************************/

        if (SETUP_BS_CHANGED == 1){
            EEPROM_writeByte(EEPROM_OLD_BS_ADDRESS, FLAG_BS);
            EEPROM_waitForWriteComplete();
            SETUP_BS_CHANGED = 0;
        }

        /*******************************************************************************************************************/
        /*                                           HAS AS SETUP BEEN CHANGED?                                            */
        /*******************************************************************************************************************/

        if (SETUP_AS_CHANGED == 1){
            EEPROM_writeByte(EEPROM_OLD_AS_ADDRESS, OPTION_AS-1);
            EEPROM_waitForWriteComplete();
            SETUP_AS_CHANGED = 0;
        }

        /*******************************************************************************************************************/
        /*                                          HAS FSR SETUP BEEN CHANGED?                                            */
        /*******************************************************************************************************************/

        if (SETUP_FSR_CHANGED == 1){

            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG4,
                                                 SETUP_FSR);

            if (error == NO_ERROR){
                EEPROM_writeByte(EEPROM_OLD_FSR_ADDRESS, SETUP_FSR);
                EEPROM_waitForWriteComplete();
                sprintf(message, "     FSR CONTROL REGISTER 4 successfully written and saved as: 0x%02X\r\n", SETUP_FSR);
                UART_PutString(message);
                SETUP_FSR_CHANGED = 0;}
            else{UART_PutString("     Error occurred during I2C comm to set control register 4\r\n");}

            if (error == NO_ERROR){ // the FIFO is reset
                error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                     LIS3DH_FIFO_CTRL_REG,
                                                     LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);}

            if (error == NO_ERROR){
                error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                     LIS3DH_FIFO_CTRL_REG,
                                                     LIS3DH_FIFO_CTRL_REG_FIFO_MODE);}

            // Resetting pointer's value on EEPROM
            UART_PutString("     Resetting EEPROM's pointer...");
            EEPROM_writeByte(0x0000, (uint8_t) EEPROM_INITIAL_ADDRESS);
            EEPROM_waitForWriteComplete();
            EEPROM_writeByte(0x0001, (uint8_t) 0);
            EEPROM_waitForWriteComplete();
            UART_PutString("     DONE\r\n");

            full_or_empty = 0;


        }

        /*******************************************************************************************************************/
        /*                                          HAS SF SETUP BEEN CHANGED?                                             */
        /*******************************************************************************************************************/

        if (SETUP_SF_CHANGED == 1){

            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG1,
                                                 SETUP_SF);

            if (error == NO_ERROR){
                EEPROM_writeByte(EEPROM_OLD_SF_ADDRESS, SETUP_SF);
                EEPROM_waitForWriteComplete();
                sprintf(message, "     SF CONTROL REGISTER 1 successfully written and saved as: 0x%02X\r\n", SETUP_SF);
                UART_PutString(message);
                SETUP_SF_CHANGED = 0;}
            else{UART_PutString("     Error occurred during I2C comm to set control register 1\r\n");}


            if (error == NO_ERROR){ // the FIFO is reset
                error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                     LIS3DH_FIFO_CTRL_REG,
                                                     LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);}

            if (error == NO_ERROR){
                error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                     LIS3DH_FIFO_CTRL_REG,
                                                     LIS3DH_FIFO_CTRL_REG_FIFO_MODE);}

            // Resetting pointer's value on EEPROM
            UART_PutString("     Resetting EEPROM's pointer...");
            EEPROM_writeByte(0x0000, (uint8_t) EEPROM_INITIAL_ADDRESS);
            EEPROM_waitForWriteComplete();
            EEPROM_writeByte(0x0001, (uint8_t) 0);
            EEPROM_waitForWriteComplete();
            UART_PutString("     DONE\r\n");

            full_or_empty = 0;

        }

        /*******************************************************************************************************************/
        /*                                                 ACQUISITION AND EEPROM WRITING                                                 */
        /*******************************************************************************************************************/
        // If "b" has been pressed
        if(StartAcquisition_flag==1){
            //If FIFO went in Overrun and we have collected 32 sensor data, we have to create packets for eeprom writing
            if(FlagFifo==1 && SensorDataReady == 1){

                index=0;
                //192 Packets creation (128 of acceleration data and 64 of sensor data)
                for(i=0; i<32; i++){

                    Packets[index] = *(AccData+i*6+4)>>6 | *(AccData+i*6+5)<<2;
                    index++;
                    Packets[index] = (*(AccData+i*6+5)>>6) | ((*(AccData+i*6+2)&0xC0)>>4) | ((*(AccData+i*6+3)&0x0F)<<4);
                    index++;
                    Packets[index] =((*(AccData+i*6+3)&0xF0)>>4) | (*(AccData+i*6)>>2) | (*(AccData+i*6+1)<<6);
                    index++;
                    Packets[index] =  AccData[i*6+1]>>2;
                    index++;
                    Packets[index] =  SensBytes_old[i*2];
                    index++;
                    Packets[index] =  SensBytes_old[i*2+1];
                    index++;

                }

                /*I Check the adress where i want to start writing*/
                EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
                EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
                EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);

                /*I start writing if we have not reached cell 0x7FFF yet */
                if (EEPROM_Adress < EEPROM_DIMENSION){
                    /*I have to write 192 Bytes = 3*64 Bytes = 3 Pages */
                    for(uint16_t t=0;t<3;t++){

                        Adress_to_write = EEPROM_Adress + t*Bytes_for_page; //During every cycle we have ti update the starting writing address
                        data_pointer = &Packets[t*Bytes_for_page]; //pointer pointing to data to be written properly updated for every cycle
                        EEPROM_writePage(Adress_to_write, data_pointer, Bytes_for_page); //Writing a 64bytes page
                        EEPROM_waitForWriteComplete();
                    }
                    //EEPROM Address update
                    EEPROM_Adress           = Adress_to_write + Bytes_for_page;
                    EEPROM_Adress_Pointer_L = (uint8_t) (EEPROM_Adress & 0xFF);
                    EEPROM_Adress_Pointer_H = (uint8_t) ((EEPROM_Adress & 0xFF00)>>8);

                    EEPROM_writeByte(0x0000, EEPROM_Adress_Pointer_L);
                    EEPROM_waitForWriteComplete();
                    EEPROM_writeByte(0x0001, EEPROM_Adress_Pointer_H);
                    EEPROM_waitForWriteComplete();
                }
                /* If EEPROM is full - LED blinking pattern */
                else{
                    full_or_empty = 1;
                    LED_EEPROM_FULL_TOGGLE();
                }

                Timer_sensor_ReadStatusRegister();         //restart the time
                //variables reset
                counter_timer=0;
                FlagFifo=0;
                SensorDataReady=0;

            }
        }

        /*******************************************************************************************************************/
        /*                                                  STREAMING                                                      */
        /*******************************************************************************************************************/

        if(StartStream_flag==1){

            EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
            EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
            EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);

            Adress_to_read = EEPROM_INITIAL_ADDRESS;

            while (Adress_to_read < EEPROM_Adress && StartStream_flag){

                UART_ClearTxBuffer(); // Clean Tx Buffer

                EEPROM_readPage(Adress_to_read, Out_Data, DATA_PACKET_DIM);

                xl = (uint8_t)  (Out_Data[2]&0x30)<<2;
                xh = (uint8_t)  (Out_Data[2]&0xC0)>>6  |  (Out_Data[3]&0x3f)<<2;
                yl = (uint8_t)  (Out_Data[1]&0x0C)<<4;
                yh = (uint8_t) ((Out_Data[1]&0xF0)>>4) | ((Out_Data[2]&0x0F)<<4);
                zl = (uint8_t)  (Out_Data[0]&0x03)<<6;
                zh = (uint8_t) ((Out_Data[0]&0xFC)>>2) |  (Out_Data[1]&0x03)<<6;

                x = (int16)((xh<<8)|(xl))>>6;               //X-axis acceleration in digit
                y = (int16)((yh <<8)|(yl))>>6;              //Y-axis acceleration in digit
                z = (int16)((zh <<8)|zl)>>6;                //Z-axis acceleration in digit

                z_uart = (int16_t) (((float32)z)*Sensitivity*0.981 + 0.5);
                y_uart = (int16_t) (((float32)y)*Sensitivity*0.981 + 0.5);
                x_uart = (int16_t) (((float32)x)*Sensitivity*0.981 + 0.5);

                UART_OutArray[1] = (uint8_t)(z_uart & 0xFF);     //LSB Z
                UART_OutArray[2] = (uint8_t)(z_uart >>8);        //MSB Z
                UART_OutArray[3] = (uint8_t)(y_uart & 0xFF);     //LSB Y
                UART_OutArray[4] = (uint8_t)(y_uart >>8);        //MSB Y
                UART_OutArray[5] = (uint8_t)(x_uart & 0xFF);     //LSB Z
                UART_OutArray[6] = (uint8_t)(x_uart >>8);        //MSB Z
                UART_OutArray[7] = Out_Data[4];                   //LSB of sensor
                UART_OutArray[8] = Out_Data[5];                   //MSB of sensor

                UART_PutArray(UART_OutArray, UART_PACKET_DIM);
                CyDelay(10);                                // 10ms

                Adress_to_read=Adress_to_read +6;

                if(Adress_to_read >= EEPROM_Adress){
                    Adress_to_read = EEPROM_INITIAL_ADDRESS;
                }
            }

        }


    /******************************************************************************************************************/
    /*                                               USER INTERFACE                                                   */
    /******************************************************************************************************************/

        if(summary_ready==1){
            INFO();
            summary_ready=0;
        }

        PB_INFO(push_button_event);
    }

}


/* [] END OF FILE */
