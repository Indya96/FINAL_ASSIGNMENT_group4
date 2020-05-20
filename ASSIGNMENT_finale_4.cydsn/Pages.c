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
#include "Pages.h"
#include "InterruptRoutines.h"

// Include required header files
#include "project.h"

void WELCOME(void){
    
    UART_PutString("===============================================================\r\n");
    UART_PutString("| /////////////// CONTROL PANNEL INSTRUCTIONS /////////////// |\r\n");
    UART_PutString("===============================================================\r\n");
    UART_PutString("|                                                             |\r\n");
    UART_PutString("|  O - summary of the latter commands entered                 |\r\n");
    UART_PutString("|                                                             |\r\n");
    UART_PutString("|  f - configuration of the accelerometer's FULL SCALE RANGE  |\r\n");
    UART_PutString("|      +--------++---------+---------+---------+---------+    |\r\n");
    UART_PutString("|      |  char  ||    1    |    2    |    3    |    4    |    |\r\n");
    UART_PutString("|      +--------++---------+---------+---------+---------+    |\r\n");
    UART_PutString("|      |  FSR   ||  +-2g   |  +-4g   |  +-8g   |  +-16g  |    |\r\n");
    UART_PutString("|      +--------++---------+---------+---------+---------+    |\r\n");
    UART_PutString("|                                                             |\r\n");
    UART_PutString("|  p - configuration of the accelerometer's SAMPLING FREQ.    |\r\n");
    UART_PutString("|      +--------++---------+---------+---------+---------+    |\r\n");
    UART_PutString("|      |  char  ||    1    |    2    |    3    |    4    |    |\r\n");
    UART_PutString("|      +--------++---------+---------+---------+---------+    |\r\n");
    UART_PutString("|      |  SF    ||   1Hz   |   10Hz  |   25Hz  |   50Hz  |    |\r\n");
    UART_PutString("|      +--------++---------+---------+---------+---------+    |\r\n");
    UART_PutString("|                                                             |\r\n");
    UART_PutString("|  v - Start EEPROM data visualization                        |\r\n");
    UART_PutString("|  u - Stop EEPROM data visualization                         |\r\n");
    UART_PutString("|  l - Additional sensors data storing                        |\r\n");
    UART_PutString("|                +--------++---------+---------+              |\r\n");
    UART_PutString("|                |  char  ||    1    |    2    |              |\r\n");
    UART_PutString("|                +--------++---------+---------+              |\r\n");
    UART_PutString("|                |  SENS  ||   LDR   |   POT   |              |\r\n");
    UART_PutString("|                +--------++---------+---------+              |\r\n");
    UART_PutString("|                                                             |\r\n");
    UART_PutString("|  b - Start acquisition from accelerometer and data storing  |\r\n");
    UART_PutString("|  s - Stop acquisition from accelerometer and data storing   |\r\n");
    UART_PutString("|                                                             |\r\n");
    UART_PutString("===============================================================\r\n");
    UART_PutString("\r\n");
    UART_PutString("WARNING!!: do NOT enter commands like 'PLF 123' but rather 'P1 L2 F3' \r\n");
    UART_PutString("example 1: if 'PLF 123' are entered, P,L and F status flags are reset. \r\n");
    UART_PutString("example 2: if 'L3 P1'   are entered, P   and L status flags are reset. \r\n");
    UART_PutString("example 3: if 'P5 2'    are entered, the sampling frequency is set to 10Hz. \r\n");
    UART_PutString("\r\n");
    UART_PutString("Enter the new command: ");
}

void INFO(void){
    
    UART_PutString(" \r\n");
    UART_PutString("Summary of the latter options entered: \r\n");
        
    switch(FLAG_BS)
    {  
        case 0:
        UART_PutString("     General state: the acquisition is NOT running \r\n");
        break;
        
        case 1:
        UART_PutString("     General state: the acquisition is running \r\n");
        break;
    }
    
    switch(OPTION_FSR)
    {
        case 0:
        UART_PutString("  Full Scale Range: NOT modified yet in this session \r\n");
        break;
        
        case 1:
        UART_PutString("  Full Scale Range: +-2g \r\n");
        break;
        
        case 2:
        UART_PutString("  Full Scale Range: +-4g \r\n");
        break;
        
        case 3:
        UART_PutString("  Full Scale Range: +-8g \r\n");
        break;
        
        case 4:
        UART_PutString("  Full Scale Range: +-16g \r\n");
        break;
    }
    
    switch(OPTION_SF)
    {
        case 0:
        UART_PutString("Sampling Frequency: NOT modified yet in this session  \r\n");
        break;
        
        case 1:
        UART_PutString("Sampling Frequency: 1Hz \r\n");
        break;
        
        case 2:
        UART_PutString("Sampling Frequency: 10Hz \r\n");
        break;
        
        case 3:
        UART_PutString("Sampling Frequency: 25Hz \r\n");
        break;
        
        case 4:
        UART_PutString("Sampling Frequency: 50Hz \r\n");
        break;
    }
    
    switch(OPTION_AS)
    {
        case 0:
        UART_PutString(" Additional Sensor: NOT modified yet in this session \r\n");
        break;
        
        case 1:
        UART_PutString(" Additional Sensor: LDR \r\n");
        break;
        
        case 2:
        UART_PutString(" Additional Sensor: Potetiometer \r\n");
        break;
    }
    
    UART_PutString(" \r\n");
    UART_PutString("Enter the new command: ");
}

/* [] END OF FILE */