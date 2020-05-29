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
    
    // control
    volatile uint8 StartAcquisition_flag ;      // begin/stop acquisition
    volatile uint8 StartStream_flag;            // begin/stop streaming data
    
    // human interface
    volatile uint8 FLAG_FSR;                    // full scale range
    volatile uint8 FLAG_SF ;                    // sampling frequency
    volatile uint8 FLAG_AS ;                    // additional sensor
    volatile uint8 FLAG_BS ;                    // begin/stop

    // option 
    volatile uint8 OPTION_FSR;                  // full scale range value
    volatile uint8 OPTION_SF ;                  // sampling frequency value
    volatile uint8 OPTION_AS ;                  // additional sensor 0 or 1
    
    // notification
    volatile uint8 summary_ready;
    volatile uint8 push_button_event;
    
    // setup 
    volatile uint8 SETUP_BS_CHANGED;  
    volatile uint8 SETUP_AS_CHANGED;
    volatile uint8 SETUP_FSR_CHANGED;  
    volatile uint8 SETUP_SF_CHANGED;
    volatile uint8 SETUP_FSR;
    volatile uint8 SETUP_SF;
    
    // pages
    void WELCOME(void);
    void INFO(void);
    void PB_INFO(uint8_t PB_event);
    void ACKNOWLEDGEMENT(void);                 // todo
    
#endif

/* [] END OF FILE */
