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
    //char     debug[192];
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
    
    // human interface
    FLAG_FSR                = 0;
    FLAG_SF                 = 0;
    FLAG_AS                 = 0;
    FLAG_BS                 = 0; 
    
    // option     
    OPTION_FSR              = 0;  
    OPTION_SF               = 0;  
    OPTION_AS               = 0;  
    
    // setup 
    SETUP_FSR_CHANGED       = 0;  
    SETUP_SF_CHANGED        = 0;
    SETUP_FSR               = 0;
    SETUP_SF                = 0;
    
    // control variables
    StartAcquisition_flag   = 0;  
    StartStream_flag        = 0;  
    
    // notification
    summary_ready           = 0;
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
    ADC_Start();
    Timer_sensor_Start(); 
   
    
    /******************************************************************************************************************/
    /*                                              INTERRUPTs ENABLE                                                 */
    /******************************************************************************************************************/
    
    CyGlobalIntEnable;                      /* Enable global interrupts. */
    isr_RX_StartEx(Custom_ISR_RX);          /* UART on RX*/
    ISR_PB_LOW_StartEx(ISR_PB_LOW);         /* Push button LOW */
    ISR_PB_HIGH_StartEx(ISR_PB_HIGH);       /* Push button HIGH */
    isr_FIFO_StartEx(Custom_FIFO_ISR);      /* FIFO interrupt enabled pointing to proper function adress. */
    isr_TIMER_StartEx(Custom_TIMER_ISR);    /* TIMER interrupt enabled pointing to proper function adress. */
    
    uint8_t  EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
    uint8_t  EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
    uint16_t EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);
                                                   
    
    /******************************************************************************************************************/
    /*                                            FIFO MODE INITIALIZATION                                            */
    /******************************************************************************************************************/
    
    UART_PutString("     Initializing...\r\n");
    
    error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                         LIS3DH_CTRL_REG3,
                                         LIS3DH_CTRL_REG3_INT_ENABLE);
    if (error == NO_ERROR){UART_PutString("     Overrun interrupt    ENABLED\r\n");}
    
    error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                         LIS3DH_FIFO_CTRL_REG,
                                         LIS3DH_FIFO_CTRL_REG_FIFO_MODE);
    if (error == NO_ERROR){UART_PutString("     FIFO mode            SELECTED\r\n");}
    
    error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                         LIS3DH_CTRL_REG5,
                                         LIS3DH_CTRL_REG5_FIFO_ENABLE);
    if (error == NO_ERROR){UART_PutString("     Fifo mode            ENABLED\r\n");}
    
    
    /******************************************************************************************************************/
    /*                                               EEPROM ... ???                                                   */
    /******************************************************************************************************************/
    
    if(EEPROM_Adress < EEPROM_INITIAL_ADDRESS){
        EEPROM_writeByte(0x0000, (uint8_t) EEPROM_INITIAL_ADDRESS);
        EEPROM_waitForWriteComplete();
    }
   
                
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
        /*                                          HAS FSR SETUP BEEN CHANGED?                                            */
        /*******************************************************************************************************************/

        if (SETUP_FSR_CHANGED == 1){
            
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG4,
                                                 SETUP_FSR);
            
            if (error == NO_ERROR){
                sprintf(message, "     FSR CONTROL REGISTER 4 successfully written as: 0x%02X\r\n", SETUP_FSR);
                UART_PutString(message);
                SETUP_FSR_CHANGED = 0;}
            else{UART_PutString("     Error occurred during I2C comm to set control register 4\r\n");}
            
            
            // TODO, salvare le impostazioni nella EEPROM

        }
        
        /*******************************************************************************************************************/
        /*                                          HAS SF SETUP BEEN CHANGED?                                             */
        /*******************************************************************************************************************/

        if (SETUP_SF_CHANGED == 1){

            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG1,
                                                 SETUP_SF);

            if (error == NO_ERROR){
                sprintf(message, "     SF CONTROL REGISTER 1 successfully written as: 0x%02X\r\n", SETUP_SF);
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
            
            // TODO, salvare le impostazioni nella EEPROM

        }
        
        
        /*******************************************************************************************************************/
        /*                                                 ACQUISITION                                                     */
        /*******************************************************************************************************************/
        if(StartAcquisition_flag){
            
            uint8_t OutArray[8]; 
            /* Sending data to EEPROM*/
            if(FlagFifo && SensorDataReady){
                    //UART_PutString("FlagFifo && SensorDataReady\r\n");
                    for(uint8_t i=0;i<32;i++)
                    {
                        Packets[index] = (uint8_t) (AccData[i*6+4]>>6) | (AccData[i*6+5]<<2);
                        index++;
                        Packets[index] = (uint8_t) (AccData[i*6+5]>>6) | (AccData[i*6+2]>>4) | (AccData[i*6+3]<<4);
                        index++;
                        //2 non 6
                        Packets[index] = (uint8_t) (AccData[i*6+3]>>4) | (AccData[i*6]>>2) | (AccData[i*6+1]<<6)  ;
                        index++;
                        Packets[index] = (uint8_t) AccData[i*6+1]>>2;
                        index++;
                        Packets[index] = (uint8_t) SensBytes_old[i*2];
                        index++;
                        Packets[index] = (uint8_t) SensBytes_old[i*2+1];
                        index++;
                        
                        // dividere i pacchetti ACCDATA e stamparli su UART
                        
                        OutArray[0]=0xA0; 
                        OutArray[1] = (uint8_t) (AccData[i*6+4]>>6) | (AccData[i*6+5]<<2);
                        OutArray[2] = (uint8_t) (AccData[i*6+5]>>6) | (AccData[i*6+2]>>4) | (AccData[i*6+3]<<4);
                        OutArray[3] = (uint8_t) (AccData[i*6+3]>>4) | (AccData[i*6]>>2) | (AccData[i*6+1]<<6)  ;
                        OutArray[4] = (uint8_t) AccData[i*6+1]>>2;
                        OutArray[5] = (uint8_t) SensBytes_old[i*2];
                        OutArray[6] = (uint8_t) SensBytes_old[i*2+1];
                        OutArray[7]=0xC0;
                        
                        UART_PutArray(OutArray, 8);

                    }

                Timer_sensor_ReadStatusRegister();         // restart the timer
              
                counter_timer   = 0;
                FlagFifo        = 0;
                SensorDataReady = 0;
            
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
