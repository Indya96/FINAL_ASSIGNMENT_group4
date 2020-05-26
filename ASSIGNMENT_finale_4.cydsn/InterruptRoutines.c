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

// Include header
#include "InterruptRoutines.h"
#include "Pages.h"

// Include required header files
#include "project.h"

/******************************************************************************************************************/
/*                                                REGISTER OPTIONS                                                */
/******************************************************************************************************************/
/*
* From datasheet:
*
*    FS[1:0]: Full-scale selection. default value: 00
*             (00: ±2 g; 01: ±4 g; 10: ±8 g; 11: ±16 g)
*
*   |************ CTRL_REG4 register **************|
*   | BDU | BLE | FS1 | FS0 | HR | ST1 | ST0 | SIM |
*   |  1  |  0  |  0  |  0  | 0  |  0  |  0  |  0  |  0x80  (±2g)
*   |  1  |  0  |  0  |  1  | 0  |  0  |  0  |  0  |  0x90  (±4g)
*   |  1  |  0  |  1  |  0  | 0  |  0  |  0  |  0  |  0xA0  (±8g)
*   |  1  |  0  |  1  |  1  | 0  |  0  |  0  |  0  |  0xB0  (±16g)
*/

#define LIS3DH_CTRL_REG4_FSR_2G  0x80
#define LIS3DH_CTRL_REG4_FSR_4G  0x90
#define LIS3DH_CTRL_REG4_FSR_8G  0xA0
#define LIS3DH_CTRL_REG4_FSR_16G 0xB0

/*
* From datasheet:
*
*   |**************** CTRL_REG1 register ****************|
*   | ODR3 | ODR2 | ODR1 | ODR0 | LPen | Zen | Yen | Xen |
*   |  0   |  0   |  0   |  1   |  0   |  1  |  1  |  1  |  0x17  (1Hz)
*   |  0   |  0   |  1   |  0   |  0   |  1  |  1  |  1  |  0x27  (10Hz)
*   |  0   |  0   |  1   |  1   |  0   |  1  |  1  |  1  |  0x37  (25Hz)
*   |  0   |  1   |  0   |  0   |  0   |  1  |  1  |  1  |  0x47  (50Hz)
*/

#define LIS3DH_CTRL_REG1_SF_1HZ   0x17
#define LIS3DH_CTRL_REG1_SF_10HZ  0x27
#define LIS3DH_CTRL_REG1_SF_25HZ  0x37
#define LIS3DH_CTRL_REG1_SF_50HZ  0x47


/******************************************************************************************************************/
/*                                            VARIABLES DECLARATION                                               */
/******************************************************************************************************************/

uint8       ch_receveid;                            // flag char 
uint8_t     PB_PRESSED           = 0;               // flag button pressed (1 means "pressed")
uint16_t    TIMER_COUNTER        = 0;               // timer counter variable
const mode  LED_ACQUISITION_ON   = {199, 100};     
const mode  LED_EXT_EEPROM_FULL  = { 49,  25};


/******************************************************************************************************************/
/*                                                TOGGLING MODES                                                  */
/******************************************************************************************************************/

static void LED_ACQUISITION_ON_TOGGLE(void){
    PWM_WritePeriod(LED_ACQUISITION_ON.Period);
    PWM_WriteCompare1(LED_ACQUISITION_ON.CMP);
}

static void LED_EXT_EEPROM_FULL_TOGGLE(void){
    PWM_WritePeriod(LED_EXT_EEPROM_FULL.Period);
    PWM_WriteCompare2(LED_EXT_EEPROM_FULL.CMP); 
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
            // todo - bloccare tutto
            WELCOME();
            break;
        
        case 'B':
        case 'b':
            FLAG_BS = 1;
            //todo
            break;
            
        case 'S':
        case 's':
            FLAG_BS = 0;
            //todo
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
