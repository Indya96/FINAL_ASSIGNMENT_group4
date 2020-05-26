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

#ifndef __PAGES_H
    #define __PAGES_H
    
    #include "cytypes.h"
    #include "stdio.h"
    
    void WELCOME(void);
    void INFO(void);
    void PB_INFO(uint8_t PB_event);
    void ACKNOWLEDGEMENT(void); // todo
    
    // declaring flags
    volatile uint8 FLAG_FSR;    // full scale range
    volatile uint8 FLAG_SF ;    // sampling frequency
    volatile uint8 FLAG_AS ;    // additional sensor
    volatile uint8 FLAG_BS ;    // begin/stop

    // declaring various mode values
    volatile uint8 OPTION_FSR;  // full scale range value
    volatile uint8 OPTION_SF ;  // sampling frequency value
    volatile uint8 OPTION_AS ;  // additional sensor 0 or 1
    
    // declaring summary flag
    volatile uint8 summary_ready;
    volatile uint8 push_button_event; 
    
    // new variables 
    volatile uint8 SETUP_FSR_CHANGED;  // nel main, se le impostazioni di FSR sono cambiate
                                       // si scrive il registro con le impostazioni "SETUP_FSR"
                                       // cambiate le impostazioni si resetta "SETUP_FSR_CHANGED"
    volatile uint8 SETUP_SF_CHANGED;
    volatile uint8 SETUP_FSR;
    volatile uint8 SETUP_SF;
    
    
#endif

/* [] END OF FILE */
