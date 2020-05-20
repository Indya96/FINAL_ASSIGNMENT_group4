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
    void ACKNOWLEDGEMENT(void); // todo
    
    // declaring flags
    volatile uint8 FLAG_FSR; // full scale range
    volatile uint8 FLAG_SF ; // sampling frequency
    volatile uint8 FLAG_AS ; // additional sensor
    volatile uint8 FLAG_BS ; // begin/stop

    // declaring various mode values
    volatile uint8 OPTION_FSR; // full scale range value
    volatile uint8 OPTION_SF ; // sampling frequency value
    volatile uint8 OPTION_AS ; // additional sensor 0 or 1
    
    // declaring summary flag
    volatile uint8 summary_ready;
    
#endif

/* [] END OF FILE */
