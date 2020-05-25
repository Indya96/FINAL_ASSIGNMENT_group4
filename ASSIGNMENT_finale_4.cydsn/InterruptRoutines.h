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

#ifndef __INTERRUPT_ROUTINES_H
    #define __INTERRUPT_ROUTINES_H
    
    #include "cytypes.h"
    #include "stdio.h"
    
    char elapsed_time_string[64];
    volatile uint16 elapsed_time;     
    
    // PWM toggling modes
    typedef struct{            
        uint16_t Period;
        uint16_t CMP; 
    } mode;
    
    // ISR PUSH BUTTON 
    CY_ISR_PROTO(ISR_PB_LOW);
    CY_ISR_PROTO(ISR_PB_HIGH);
    
    // ISR UART RX
    CY_ISR_PROTO (Custom_ISR_RX);
 
#endif

/* [] END OF FILE */
