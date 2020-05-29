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
    
    #define FIFO_DATA_DIM   192 // 6 output registers of 32 levels
    #define SENSOR_DATA_DIM 32  // the fifo stores 32 values of acceleration 
                                // so we need 23 values of the sensor (potentiometer/photoresistor)

    volatile   uint8_t  AccData[FIFO_DATA_DIM];
               uint8_t  AccData2[FIFO_DATA_DIM];
    volatile   uint8_t  FlagFifo;
    volatile   uint8_t  counter_timer;
    volatile   uint8_t  CountSensor;
    volatile   uint16_t SensData[SENSOR_DATA_DIM];
    volatile   uint8_t  SensBytes[2*SENSOR_DATA_DIM];
    volatile   uint8_t  SensBytes_old[2*SENSOR_DATA_DIM];
    volatile   uint8_t  SensorDataReady; 
    volatile   uint8_t  counter_timer;   
    volatile   uint8_t  SensorDataReady; 
    
    char elapsed_time_string[64];
    volatile uint16 elapsed_time;     
    
    // PWM toggling options
    typedef struct{            
        uint16_t Period;
        uint16_t CMP; 
    } mode;
    
    void LED_ACQUISITION_ON_TOGGLE(void);
    void LED_ACQUISITION_OFF_TOGGLE(void);
    void LED_EXT_EEPROM_EMPTY_TOGGLE(void);
    void LED_EXT_EEPROM_FULL_TOGGLE(void);
    void CHANGE_ADC_SF(uint16_t period);
    
    CY_ISR_PROTO(ISR_PB_LOW);           // ISR PUSH BUTTON LOW
    CY_ISR_PROTO(ISR_PB_HIGH);          // ISR PUSH BUTTON HIGH
    CY_ISR_PROTO(Custom_ISR_RX);        // ISR UART RX
    CY_ISR_PROTO(Custom_FIFO_ISR);      // ISR FIFO overflow
    CY_ISR_PROTO(Custom_TIMER_ISR);     // ISR TIMER overflow

#endif

/* [] END OF FILE */
