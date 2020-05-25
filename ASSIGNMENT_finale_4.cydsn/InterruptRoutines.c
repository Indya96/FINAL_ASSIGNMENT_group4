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
            
        case '1':
            if(FLAG_FSR==1 && FLAG_SF==0 && FLAG_AS==0){ // FULL SCALE RANGE
                FLAG_FSR  = 0;
                OPTION_FSR = 1;
                //todo
            }
            if(FLAG_FSR==0 && FLAG_SF==1 && FLAG_AS==0){ // SAMPLING FREQUENCY
                FLAG_SF  = 0;
                OPTION_SF = 1;
                //todo
            }
            if(FLAG_FSR==0 && FLAG_SF==0 && FLAG_AS==1){ // Additional sensors
                FLAG_AS  = 0;
                OPTION_AS = 1;
                //todo
            }
            break;
        
        case '2':
            if(FLAG_FSR==1 && FLAG_SF==0 && FLAG_AS==0){ // FULL SCALE RANGE
                FLAG_FSR  = 0;
                OPTION_FSR = 2;
                //todo
            }
            if(FLAG_FSR==0 && FLAG_SF==1 && FLAG_AS==0){ // SAMPLING FREQUENCY
                FLAG_SF  = 0;
                OPTION_SF = 2;
                //todo
            }
            if(FLAG_FSR==0 && FLAG_SF==0 && FLAG_AS==1){ // Additional sensors
                FLAG_AS  = 0;
                OPTION_AS = 2;
                //todo
            }
            break;
            
        case '3':
            if(FLAG_FSR==1 && FLAG_SF==0){ // FULL SCALE RANGE
                FLAG_FSR  = 0;
                OPTION_FSR = 3;
                //todo
            }
            if(FLAG_FSR==0 && FLAG_SF==1){ // SAMPLING FREQUENCY
                FLAG_SF  = 0;
                OPTION_SF = 3;
                //todo
            }
            break;
            
        case '4':
            if(FLAG_FSR==1 && FLAG_SF==0){ // FULL SCALE RANGE
                FLAG_FSR  = 0;
                OPTION_FSR = 4;
                //todo
            }
            if(FLAG_FSR==0 && FLAG_SF==1){ // SAMPLING FREQUENCY
                FLAG_SF  = 0;
                OPTION_SF = 4;
                //todo
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
            elapsed_time = 5*(60000-TIMER_COUNTER);///200;
            sprintf(elapsed_time_string, "     Elapsed time: %dms \r\n", elapsed_time);
            // TODO
        }
        
        // if 5 seconds haven't been elapsed
        if(TIMER_COUNTER > 59000){
            push_button_event = 2;
            elapsed_time = 5*(60000-TIMER_COUNTER);///200;
            sprintf(elapsed_time_string, "     Elapsed time: %dms \r\n", elapsed_time);
            // TODO
        }
        
        Timer_PB_Stop();
    } 
} 
/* [] END OF FILE */
