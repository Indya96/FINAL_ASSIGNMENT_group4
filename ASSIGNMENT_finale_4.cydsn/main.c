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
#include "project.h"
#include "stdio.h"
#include "InterruptRoutines.h"
#include "Pages.h"

int main(void)
{
    // variable initialization
    FLAG_FSR            = 0;  // full scale range
    FLAG_SF             = 0;  // sampling frequency
    FLAG_AS             = 0;  // additional sensor
    FLAG_BS             = 0;  // begin/stop
    OPTION_FSR          = 0;  // full scale range value
    OPTION_SF           = 0;  // sampling frequency value
    OPTION_AS           = 0;  // additional sensor 0 or 1
    
    summary_ready       = 0;
    push_button_event   = 0;
    
    CyGlobalIntEnable; // Enable global interrupts.
    
    // Start the components
    UART_Start();
    isr_RX_StartEx(Custom_ISR_RX);
    PWM_Start();
    ISR_PB_LOW_StartEx(ISR_PB_LOW);
    ISR_PB_HIGH_StartEx(ISR_PB_HIGH);
    
    // welcome page
    WELCOME();
    
    for(;;)
    {
        if(summary_ready==1){  
            INFO();        
            summary_ready=0;
        }
    
        PB_INFO(push_button_event); 
    }
}
/* [] END OF FILE */
