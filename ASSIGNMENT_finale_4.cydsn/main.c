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
   
    char     message[50];
             SensorDataReady                    = 0;    
    uint8_t  Packets[192];
    uint8_t  index                              = 0;
    uint8_t  OutputData[DATA_PACKET_DIM];
    uint16_t EEPROM_read_address                = EEPROM_INITIAL_ADDRESS;
    int16_t  Z_acc_digit;
    int16_t  Y_acc_digit;
    int16_t  X_acc_digit;
    int16_t  Z_acc_mg;
    int16_t  Y_acc_mg;
    int16_t  X_acc_mg;
    int16    sensitivity                        = 1;  
    uint8_t  UART_OutArray[UART_PACKET_DIM];
    UART_OutArray[0]                            = 0xA0; //header
    UART_OutArray[UART_PACKET_DIM - 1]          = 0xC0; //tail
    ErrorCode error;
    
    uint8_t OLD_FSR;
    uint8_t OLD_SF;
    uint8_t OLD_AS;
    uint8_t OLD_BS;
    
    FLAG_FSR                = 0;        // human interface
    FLAG_SF                 = 0;
    FLAG_AS                 = 0;
    FLAG_BS                 = 0; 
    OPTION_FSR              = 0;        // option   
    OPTION_SF               = 0;  
    OPTION_AS               = 0;  
    SETUP_FSR_CHANGED       = 0;        // setup 
    SETUP_SF_CHANGED        = 0;
    SETUP_AS_CHANGED        = 0;
    SETUP_BS_CHANGED        = 0;
    SETUP_FSR               = 0;
    SETUP_SF                = 0;
    StartAcquisition_flag   = 0;        // control variables
    StartStream_flag        = 0;  
    summary_ready           = 0;        // notification
    push_button_event       = 0;
    
    
    /******************************************************************************************************************/
    /*                                               COMPONENTS START                                                 */
    /******************************************************************************************************************/
    
    UART_Start();
    SPIM_1_Start();
    I2C_Peripheral_Start();
    CyDelay(10);
    PWM_Start();
    PWM_EXT_Start();
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
                                         
    
    /******************************************************************************************************************/
    /*                                            FIFO MODE INITIALIZATION                                            */
    /******************************************************************************************************************/
    
    UART_PutString("     Initializing...\r\n");
    
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
    /*                                               EEPROM ... ???                                                   */
    /******************************************************************************************************************/
    
    uint8_t  EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
    uint8_t  EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
    uint16_t EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);
    uint8_t  SETUP_CORRUPTED = 0;
    
    if(EEPROM_Adress < EEPROM_INITIAL_ADDRESS){
        EEPROM_writeByte(0x0000, (uint8_t) EEPROM_INITIAL_ADDRESS);
        EEPROM_waitForWriteComplete();
    }
    
    /******************************************************************************************************************/
    /*                                            RETRIVING OLD SETUP                                                 */
    /******************************************************************************************************************/
    
     /* non c'è scritto niente riguardo la scrittura dei dati dall'ultimo punto in cui ci si era fermati, quindi
       si potrebbe ricominciare da 0x80 */
    
    UART_PutString("     Retriving old setup...\r\n");
    
    // OLD Sampling Frequency ----------------------------------------------------------------------------
    OLD_SF = EEPROM_readByte(EEPROM_OLD_SF_ADDRESS);
    if (OLD_SF!=0x17 && OLD_SF!=0x27 && OLD_SF!=0x37 && OLD_SF!=0x47){
                            UART_PutString("     Old Sampling frequency            CORRUPTED\r\n");
        SETUP_CORRUPTED++;}
    else{                  
                            UART_PutString("     Old additional sensor             RETRIVED\r\n");
                          sprintf(message, "                                       0x%02X\r\n", OLD_SF);
    UART_PutString(message);}
    
    
    // OLD Full Scale Range ------------------------------------------------------------------------------
    OLD_FSR = EEPROM_readByte(EEPROM_OLD_FSR_ADDRESS);
    if (OLD_FSR!=0x80 && OLD_FSR!=0x90 && OLD_FSR!=0xA0 && OLD_FSR!=0xB0){
                            UART_PutString("     Old full scale range              CORRUPTED\r\n");
        SETUP_CORRUPTED++;}
    else{                  
                            UART_PutString("     Old full scale range              RETRIVED\r\n");
                          sprintf(message, "                                       0x%02X\r\n", OLD_FSR);
    UART_PutString(message);}
 
    
    // OLD Additional sensor ----------------------------------------------------------------------------
    OLD_AS = EEPROM_readByte(EEPROM_OLD_AS_ADDRESS);
    if (OLD_AS!=0x00 && OLD_AS!=0x01){
                            UART_PutString("     Old additional sensor             CORRUPTED\r\n");
        SETUP_CORRUPTED++;}                    
    else{                  
                            UART_PutString("     Old additional sensor             RETRIVED\r\n");
                          sprintf(message, "                                       0x%02X\r\n", OLD_AS);
        UART_PutString(message);}
    
    // OLD Start/Stop Condition -------------------------------------------------------------------------
    OLD_BS = EEPROM_readByte(EEPROM_OLD_BS_ADDRESS);
    if (OLD_BS!=0x00 && OLD_BS!=0x01){
                            UART_PutString("     Old start/stop condition          CORRUPTED\r\n");
        SETUP_CORRUPTED++;}       
    else{                  
                            UART_PutString("     Old start/stop condition          RETRIVED\r\n");
                          sprintf(message, "                                       0x%02X\r\n", OLD_BS);
        UART_PutString(message);}
    
    /******************************************************************************************************************/
    /*                                        CHECKING OLD SETUP INTEGRITY                                            */
    /******************************************************************************************************************/
    /*  IF OLD SETUP ARE CORRUPTED, DEFAULT MODE  */
    /**********************************************/
    
    if(SETUP_CORRUPTED>0){
        
        // default Start/Stop condition
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
        LED_ACQUISITION_OFF_TOGGLE();
        UART_PutString("     DEFAULT Start/Stop cond.   : STOP \r\n");
        
        // default sampling frequency
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG1,
                                             LIS3DH_CTRL_REG1_SF_1HZ);
        if (error == NO_ERROR){
            UART_PutString("     DEFAULT sampling frequency : 1Hz \r\n");
            CHANGE_ADC_SF(1000);}
        else{UART_PutString("     Error occurred setting the default sampling frequency \r\n");}
        
        // default full scale range
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG4,
                                             LIS3DH_CTRL_REG4_FSR_2G);
        if (error == NO_ERROR){
            UART_PutString("     DEFAULT full scale range   : +-2g \r\n");}
        else{UART_PutString("     Error occurred setting the default full scale range \r\n");}
        
        // default additional sensor
        AMux_Select(0);
        UART_PutString("     DEFAULT additional sensor  : Potentiometer \r\n");
        
        // saving latter setup on EEPROM
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
        
        switch(OLD_SF){ 
            case LIS3DH_CTRL_REG1_SF_1HZ:  CHANGE_ADC_SF(1000); break;
            case LIS3DH_CTRL_REG1_SF_10HZ: CHANGE_ADC_SF(100);  break;
            case LIS3DH_CTRL_REG1_SF_25HZ: CHANGE_ADC_SF(40);   break;        
            case LIS3DH_CTRL_REG1_SF_50HZ: CHANGE_ADC_SF(20);   break;}
        
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG1,
                                             OLD_SF);
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_CTRL_REG4,
                                             OLD_FSR);
        
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_FIFO_CTRL_REG,
                                             LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);
        error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             LIS3DH_FIFO_CTRL_REG,
                                             LIS3DH_FIFO_CTRL_REG_FIFO_MODE);
        
        if(OLD_AS==0){AMux_Select(1);} // LDR
        if(OLD_AS==1){AMux_Select(0);} // POT
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
            LED_ACQUISITION_OFF_TOGGLE();
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
            LED_ACQUISITION_ON_TOGGLE();
        }
    
    }
    
                
    /******************************************************************************************************************/
    /*                                                 WELCOME PAGE                                                   */
    /******************************************************************************************************************/
    
                                                        WELCOME();     

    
    /******************************************************************************************************************/
    /*                                                INFINITE CYCLE                                                  */
    /******************************************************************************************************************/
    int16           x;
    int16           y;
    int16           z;
    uint16          sensore;
    uint16_t        indirizzo;
    uint8_t         value;
    uint8_t         dato0;                                      
    uint8_t         dato1;
    uint8_t         dato2;
    uint8_t         dato3;
    uint8_t         dato4;
    uint8_t         dato5;                                        
    char            messaggio[100];
    
    for(;;)
    {
        
        /*******************************************************************************************************************/
        /*                                           HAS AS SETUP BEEN CHANGED?                                            */
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
            
            LED_EXT_EEPROM_EMPTY_TOGGLE();
            
            // TODO, cancellare i dati dei sensori nella EEPROM
            // TODO, resettare il puntatore della EEPROM per ricominciare a salvare dati dall'inizio

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
            
            LED_EXT_EEPROM_EMPTY_TOGGLE();
            
            // TODO, cancellare i dati dei sensori nella EEPROM
            // TODO, resettare il puntatore della EEPROM per ricominciare a salvare dati dall'inizio

        }
        
        
        /*******************************************************************************************************************/
        /*                                                 ACQUISITION                                                     */
        /*******************************************************************************************************************/
        
        if(StartAcquisition_flag==1){

            if(FlagFifo==1 && SensorDataReady == 1){
                
                UART_PutString(" interrupt\r\n");
                for(uint8_t i=0;i<32;i++){
                 
                    Packets[index] = (AccData[i*6+4]>>6) | (AccData[i*6+5]<<2);
                    index++;
                    Packets[index] = (AccData[i*6+5]>>6) | ((AccData[i*6+2]&0xC0)>>4) | ((AccData[i*6+3]&=0x0F)<<4);
                    index++;
                    Packets[index] = ((AccData[i*6+3]&0xF0)>>4) | (AccData[i*6]>>2) | (AccData[i*6+1]<<6)  ;
                    index++;
                    Packets[index] =  AccData[i*6+1]>>2;
                    index++;
                    Packets[index] =  SensBytes_old[i*2];
                    index++;
                    Packets[index] =  SensBytes_old[i*2+1];
                    index++; 

                    z = (int16)((AccData[i*6+5] <<8)|(AccData[i*6+4]))>>6; //Z-axis acceleration in digit
                    y = (int16)((AccData[i*6+3] <<8)|(AccData[i*6+2]))>>6; //Z-axis acceleration in digit
                    x = (int16)((AccData[i*6+1] <<8)|(AccData[i*6]))>>6;   //Z-axis acceleration in digit

                    z = z*4;
                    y = y*4;
                    x = x*4;
                    
                    sensore = (uint16) SensBytes_old[i*2+1] <<8 | SensBytes_old[i*2];
                    sprintf(messaggio,"Z:%d \n Y:%d \n X:%d \n Sensore: %d\n",z,y,x,sensore);
                    UART_PutString(messaggio);
                }
                
                /*I Check the adress where i want to start writing*/
                EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
                EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
                EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);
                
                /*I have to write 192 Bytes = 3*64 Bytes = 3 Pages */
                if (EEPROM_Adress != EEPROM_DIMENSION)
                {
                    for(uint16_t t=0;t<192;t++)
                    {
                        indirizzo=EEPROM_Adress+t;
                        value=Packets[t];
                        EEPROM_writeByte(indirizzo,value);
                        EEPROM_waitForWriteComplete();
                    }
                    
                    EEPROM_Adress = indirizzo+1;
                    sprintf(messaggio,"Indirizzo nuovo: %d\n",EEPROM_Adress);
                    UART_PutString(messaggio);
                    EEPROM_Adress_Pointer_L = (uint8_t) (EEPROM_Adress & 0xFF);
                    EEPROM_Adress_Pointer_H = (uint8_t) ((EEPROM_Adress & 0xFF00)>>8);
                    EEPROM_writeByte(0x0000, EEPROM_Adress_Pointer_L);
                    EEPROM_waitForWriteComplete();
                    EEPROM_writeByte(0x0001, EEPROM_Adress_Pointer_H);
                    EEPROM_waitForWriteComplete();
                    sprintf(messaggio,"FINE SCRITTURA\n");
                }
                else
                {
                    LED_EXT_EEPROM_FULL_TOGGLE();
                }
                
                sprintf(messaggio,"INIZIO LETTURA\n");
                indirizzo=EEPROM_INITIAL_ADDRESS;
                
                while(indirizzo<EEPROM_Adress && StartAcquisition_flag==1){ // StartAcquisition_flag==1 per debug
                    
                    dato0 = EEPROM_readByte(indirizzo);
                    indirizzo++;
                    dato1 = EEPROM_readByte(indirizzo);
                    indirizzo++;
                    dato2 = EEPROM_readByte(indirizzo);
                    indirizzo++;
                    dato3 = EEPROM_readByte(indirizzo);
                    indirizzo++;
                    dato4 = EEPROM_readByte(indirizzo);
                    indirizzo++;
                    dato5 = EEPROM_readByte(indirizzo);
                    indirizzo++;
                    
                    z = (int16)((dato1 & 0x03)<<8  | dato0);       //Z-axis acceleration in digit
                    y = (int16)((dato2 & 0x0F)<<6) |(dato1>>2);    //Y-axis acceleration in digit
                    x = (int16) (dato3<<4)|(dato2>>4);             //X-axis acceleration in digit
                    sensore = (uint16_t) (dato4 | dato5<<8);

                    sprintf(messaggio," Z:%d \n Y:%d \n X:%d \n Sensore: %d\n Cella A cui punto: %d \n",z*4,y*4,x*4,sensore,indirizzo);
                    UART_PutString(messaggio);
                }
                
                UART_PutString("FINE LETTURA");
                
                Timer_sensor_ReadStatusRegister();         //restart the time

                counter_timer   = 0;
                FlagFifo        = 0;
                SensorDataReady = 0;
                UART_PutString("fine interrupt\r\n");
            }
        }
        
        /*******************************************************************************************************************/
        /*                                                  STREAMING                                                      */
        /*******************************************************************************************************************/
        
        if(StartStream_flag==1){
                         
            EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
            EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
            EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);

            if (EEPROM_read_address < EEPROM_Adress)
            {
              
                EEPROM_readPage(EEPROM_read_address,OutputData,DATA_PACKET_DIM);
                   
                Z_acc_digit= (int16)((OutputData[1] & 0x03)<<8|OutputData[0]);                  //Z-axis acceleration in digit
                Y_acc_digit= (int16)((OutputData[2] & 0x0F)<<6|((OutputData[1]>>2) & 0x3F));    //Y-axis acceleration in digit
                X_acc_digit= (int16)((OutputData[3] & 0x3F)<<4|((OutputData[2]>>4) & 0x0F));    //X-axis acceleration in digit
                  
                Z_acc_mg = Z_acc_digit * sensitivity;
                Y_acc_mg = Y_acc_digit * sensitivity;
                X_acc_mg = X_acc_digit * sensitivity;
                    
                UART_OutArray[1] = (uint8_t)(Z_acc_mg & 0xFF);  //LSB Z
                UART_OutArray[2] = (uint8_t)(Z_acc_mg >>8);     //MSB Z
                UART_OutArray[3] = (uint8_t)(Y_acc_mg & 0xFF);  //LSB Y
                UART_OutArray[4] = (uint8_t)(Y_acc_mg >>8);     //MSB Y
                UART_OutArray[5] = (uint8_t)(X_acc_mg & 0xFF);  //LSB Z
                UART_OutArray[6] = (uint8_t)(X_acc_mg >>8);     //MSB Z
                UART_OutArray[7] = OutputData[4];               //LSB of sensor
                UART_OutArray[8] = OutputData[5];               //MSB of sensor
            
                UART_PutArray(UART_OutArray, UART_PACKET_DIM);   
                EEPROM_read_address = EEPROM_read_address + 6;

            }  
            
            if(EEPROM_read_address >= EEPROM_Adress)
            {
                EEPROM_read_address = EEPROM_INITIAL_ADDRESS;       
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
