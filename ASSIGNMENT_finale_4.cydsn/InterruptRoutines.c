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

#include "InterruptRoutines.h"
#include "I2C_Interface.h"
#include "SPI_Interface.h"
#include "project.h"
#include "Registers.h"
#include "Pages.h"


/******************************************************************************************************************/
/*                                            VARIABLES DECLARATION                                               */
/******************************************************************************************************************/

uint8       ch_receveid;                             // flag char 
uint8_t     PB_PRESSED            = 0;               // flag button pressed (1 means "pressed")
uint16_t    TIMER_COUNTER         = 0;               // timer counter variable
ErrorCode   error;

const mode  LED_ACQUISITION_ON    = {199, 100};    
const mode  LED_ACQUISITION_OFF   = {199,  0};    
const mode  LED_EXT_EEPROM_FULL   = { 49,  25};
const mode  LED_EXT_EEPROM_EMPTY  = { 49,  0};

/******************************************************************************************************************/
/*                                            CHANGE ADC SAMPLING FREQ.                                           */
/******************************************************************************************************************/

void CHANGE_ADC_SF(uint16_t period){
    
    Timer_sensor_Stop();
    Timer_sensor_ReadStatusRegister();
    Timer_sensor_WritePeriod(period);
    Timer_sensor_ReadStatusRegister();
    Timer_sensor_Start();

}

/******************************************************************************************************************/
/*                                               CUSTOM FIFO ISR                                                  */
/******************************************************************************************************************/

CY_ISR(Custom_FIFO_ISR)
{
    //UART_PutString("FIFO OVR ISR \r\n");
    //Reading data from FIFO and storing data in AccData array;
    ErrorCode error = I2C_Peripheral_ReadRegisterMulti(LIS3DH_DEVICE_ADDRESS,
                                                       LIS3DH_OUT_X_L,
                                                       FIFO_DATA_DIM,
                                                       &AccData2[0]);
        if (error==NO_ERROR){

            for (int t=0; t<FIFO_DATA_DIM; t++){
                AccData[t] = AccData2[t];}


            if (error == NO_ERROR){
                error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                     LIS3DH_FIFO_CTRL_REG,
                                                     LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);}
            
            if (error == NO_ERROR){
                error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                     LIS3DH_FIFO_CTRL_REG,
                                                     LIS3DH_FIFO_CTRL_REG_FIFO_MODE);}

            FlagFifo = 1;
            
    }
    
}

/******************************************************************************************************************/
/*                                              CUSTOM TIMER ISR                                                  */
/******************************************************************************************************************/

CY_ISR(Custom_TIMER_ISR)
{
    Timer_sensor_ReadStatusRegister();         //restart the time
    if(counter_timer<(SENSOR_DATA_DIM))
    {
        SensData[counter_timer]= ADC_Read32();  //read data from ADC
        /* verify the value */
        if (SensData[counter_timer] < 0)        SensData[counter_timer] = 0;
        if (SensData[counter_timer] > 65535)    SensData[counter_timer] = 65535;
        
        SensBytes[2*counter_timer]     = (uint8_t)(SensData[counter_timer] & 0xFF);  //LSB
        SensBytes[2*counter_timer + 1] = (uint8_t)(SensData[counter_timer]>>8) ;     //MSB
        
        counter_timer ++;
    }
    
    else if(counter_timer == (SENSOR_DATA_DIM))
    {
        int i=0;
        for(i=0;i<SENSOR_DATA_DIM*2;i++)
        {
            SensBytes_old[i]=SensBytes[i];
        }
        counter_timer=0;
        SensorDataReady=1;
    }
       
}

/******************************************************************************************************************/
/*                                                TOGGLING MODES                                                  */
/******************************************************************************************************************/

void LED_ACQUISITION_ON_TOGGLE(void){
    PWM_WritePeriod(LED_ACQUISITION_ON.Period);
    PWM_WriteCompare(LED_ACQUISITION_ON.CMP);
}

void LED_ACQUISITION_OFF_TOGGLE(void){
    PWM_WritePeriod(LED_ACQUISITION_OFF.Period);
    PWM_WriteCompare(LED_ACQUISITION_OFF.CMP);
}

void LED_EXT_EEPROM_FULL_TOGGLE(void){
    PWM_EXT_WritePeriod(LED_EXT_EEPROM_FULL.Period);
    PWM_EXT_WriteCompare(LED_EXT_EEPROM_FULL.CMP); 
}

void LED_EXT_EEPROM_EMPTY_TOGGLE(void){
    PWM_EXT_WritePeriod(LED_EXT_EEPROM_EMPTY.Period);
    PWM_EXT_WriteCompare(LED_EXT_EEPROM_EMPTY.CMP); 
}

/******************************************************************************************************************/
/*                                              UART RX INTERRUPT                                                 */
/******************************************************************************************************************/

CY_ISR(Custom_ISR_RX)
{
    /*******************************/
    /*         INPUT CHECK         */
    /*******************************/
    
    // Non-blocking call to get the latest data recieved
    ch_receveid = UART_GetChar();
        
    // Set flags based on UART command
    switch(ch_receveid){
        
        case '?':
            StartStream_flag = 0;
            WELCOME();
            break;
        
        case 'B':
        case 'b':
            UART_PutString("\r\n");
            StartAcquisition_flag = 1;
            FLAG_BS = 1;
            Timer_sensor_ReadStatusRegister();
            Timer_sensor_Start();
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
            UART_PutString("\r\n");
            LED_ACQUISITION_ON_TOGGLE();
            break;
            
        case 'S':
        case 's':
            UART_PutString("\r\n");
            StartAcquisition_flag = 0;
            FLAG_BS = 0;
            Timer_sensor_Stop();
            Timer_sensor_ReadStatusRegister();
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG3,
                                                 LIS3DH_CTRL_REG3_INT_DISABLED);
            if (error == NO_ERROR){UART_PutString("     Overrun interrupt    DISABLED\r\n");}
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_FIFO_CTRL_REG,
                                                 LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);
            if (error == NO_ERROR){UART_PutString("     BYPASS mode          SELECTED\r\n");}
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG5,
                                                 LIS3DH_CTRL_REG5_FIFO_DISABLE);
            if (error == NO_ERROR){UART_PutString("     FIFO mode            DISABLED\r\n");}
            UART_PutString("\r\n");
            LED_ACQUISITION_OFF_TOGGLE();
            break;
            
        case 'F':
        case 'f':
            FLAG_FSR = 1;
            break;
            
        case 'P':
        case 'p':
            FLAG_SF = 1;
            break;
            
        case 'L':
        case 'l':
            FLAG_AS = 1;
            break;
        
        case 'V':
        case 'v':
            StartAcquisition_flag = 0;
            FLAG_BS = 0;
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG3,
                                                 LIS3DH_CTRL_REG3_INT_DISABLED);
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_FIFO_CTRL_REG,
                                                 LIS3DH_FIFO_CTRL_REG_BYPASS_MODE);
            error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                                 LIS3DH_CTRL_REG5,
                                                 LIS3DH_CTRL_REG5_FIFO_DISABLE);
            Timer_sensor_Stop();
            Timer_sensor_ReadStatusRegister();
            LED_ACQUISITION_OFF_TOGGLE();
            StartStream_flag = 1;
            break;
            
        case 'U':
        case 'u':
            StartStream_flag = 0;
            break;
        
        case '0':
            if(FLAG_AS==1){
                FLAG_AS   = 0;
                OPTION_AS = 1;
                AMux_Select(1);   // LDR               
            }
            break;
            
        case '1':
            if(FLAG_FSR==1 && FLAG_SF==0 && FLAG_AS==0){
                FLAG_FSR            = 0;
                OPTION_FSR          = 1;
                SETUP_FSR           = LIS3DH_CTRL_REG4_FSR_2G;
                SETUP_FSR_CHANGED   = 1;
            }
            if(FLAG_FSR==0 && FLAG_SF==1 && FLAG_AS==0){
                FLAG_SF            = 0;
                OPTION_SF          = 1;
                SETUP_SF           = LIS3DH_CTRL_REG1_SF_1HZ;    
                SETUP_SF_CHANGED   = 1;
                CHANGE_ADC_SF(1000);
            }
            if(FLAG_FSR==0 && FLAG_SF==0 && FLAG_AS==1){ 
                FLAG_AS   = 0;
                OPTION_AS = 2;
                AMux_Select(0);   // POT      
            }
            break;
        
        case '2':
            if(FLAG_FSR==1 && FLAG_SF==0 && FLAG_AS==0){
                FLAG_FSR            = 0;
                OPTION_FSR          = 2;
                SETUP_FSR           = LIS3DH_CTRL_REG4_FSR_4G;
                SETUP_FSR_CHANGED   = 1;
            }
            if(FLAG_FSR==0 && FLAG_SF==1 && FLAG_AS==0){
                FLAG_SF            = 0;
                OPTION_SF          = 2;
                SETUP_SF           = LIS3DH_CTRL_REG1_SF_10HZ;
                SETUP_SF_CHANGED   = 1;
                CHANGE_ADC_SF(100);
            }
            break;
            
        case '3':
            if(FLAG_FSR==1 && FLAG_SF==0){
                FLAG_FSR            = 0;
                OPTION_FSR          = 3;
                SETUP_FSR           = LIS3DH_CTRL_REG4_FSR_8G;
                SETUP_FSR_CHANGED   = 1;
            }
            if(FLAG_FSR==0 && FLAG_SF==1){
                FLAG_SF            = 0;
                OPTION_SF          = 3;
                SETUP_SF           = LIS3DH_CTRL_REG1_SF_25HZ;
                SETUP_SF_CHANGED   = 1;
                CHANGE_ADC_SF(40);
            }
            break;
            
        case '4':
            if(FLAG_FSR==1 && FLAG_SF==0){ 
                FLAG_FSR            = 0;
                OPTION_FSR          = 4;
                SETUP_FSR           = LIS3DH_CTRL_REG4_FSR_16G;
                SETUP_FSR_CHANGED   = 1;
            }
            if(FLAG_FSR==0 && FLAG_SF==1){ 
                FLAG_SF            = 0;
                OPTION_SF          = 4;
                SETUP_SF           = LIS3DH_CTRL_REG1_SF_50HZ;
                SETUP_SF_CHANGED   = 1;
                CHANGE_ADC_SF(20);
            }
            break;
         
        case 'O':
        case 'o':
            summary_ready = 1;
            break;
            
        default:
            break;    
    }
    
    /*******************************/
    /*     INTEGRITY CONTROL       */
    /*******************************/
    
    if(FLAG_FSR==1 && FLAG_SF==1){
        FLAG_FSR=0;
        FLAG_SF=0;
    }
    
    if(FLAG_AS==1 && FLAG_SF==1){
        FLAG_AS=0;
        FLAG_SF=0;
    }
    
    if(FLAG_AS==1 && FLAG_FSR==1){
        FLAG_AS=0;
        FLAG_FSR=0;
    }
    
}

/******************************************************************************************************************/
/*                                             PUSH BUTTON INTERRUPTs                                             */
/******************************************************************************************************************/

CY_ISR(ISR_PB_LOW){  
    PB_PRESSED = 1;     // flag 1 if the button is pressed
    Timer_PB_Start();   // timer start
} 

CY_ISR(ISR_PB_HIGH){  
    if(PB_PRESSED == 1){                            // if the button has been pressed
        
        TIMER_COUNTER = Timer_PB_ReadCounter();     // read the time elapsed
        Timer_PB_WriteCounter(0);                   // reset time elapsed
        PB_PRESSED = 0;          
        
        // if 5 seconds have been elapsed
        if(TIMER_COUNTER < 59000){ 
            push_button_event = 1;
            elapsed_time = 5*(60000-TIMER_COUNTER);
            sprintf(elapsed_time_string, "     Elapsed time: %dms \r\n", elapsed_time);
            // TODO
        }
        
        // if 5 seconds haven't been elapsed
        if(TIMER_COUNTER > 59000){
            push_button_event = 2;
            elapsed_time = 5*(60000-TIMER_COUNTER);
            sprintf(elapsed_time_string, "     Elapsed time: %dms \r\n", elapsed_time);
            // TODO
        }
        
        Timer_PB_Stop();
    } 
} 
/* [] END OF FILE */
