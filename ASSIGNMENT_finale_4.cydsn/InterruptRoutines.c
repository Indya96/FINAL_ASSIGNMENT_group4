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

// Variables declaration
uint8 ch_receveid;

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
/* [] END OF FILE */
