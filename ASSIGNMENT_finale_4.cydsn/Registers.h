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

/* [] END OF FILE */

#ifndef __REGISTERS_H
    #define __REGISTERS_H
    
    #define EEPROM_PAGE_DIM                     64
    #define EEPROM_DIMENSION                    0x7FFF
    #define EEPROM_INITIAL_ADDRESS              0x80
    #define EEPROM_OLD_SF_ADDRESS               0x0002
    #define EEPROM_OLD_FSR_ADDRESS              0x0003
    #define EEPROM_OLD_AS_ADDRESS               0x0004
    #define EEPROM_OLD_BS_ADDRESS               0x0005
    
    #define ACC_PACKET_DIM                      4
    #define SENS_PACKET_DIM                     2
    #define DATA_PACKET_DIM                     ACC_PACKET_DIM + SENS_PACKET_DIM 
    #define UART_PACKET_DIM                     10

    #define LIS3DH_DEVICE_ADDRESS               0x18
    #define LIS3DH_OUT_X_L                      0x28
    
    #define LIS3DH_CTRL_REG5                    0x24
    #define LIS3DH_CTRL_REG5_FIFO_ENABLE        0x40
    #define LIS3DH_CTRL_REG5_FIFO_DISABLE       0x00
    
    #define LIS3DH_FIFO_CTRL_REG                0x2E
    #define LIS3DH_FIFO_CTRL_REG_BYPASS_MODE    0x00
    #define LIS3DH_FIFO_CTRL_REG_FIFO_MODE      0x40
    
    #define LIS3DH_CTRL_REG3                    0x22
    #define LIS3DH_CTRL_REG3_INT_ENABLE         0x02
    #define LIS3DH_CTRL_REG3_INT_DISABLED       0x00
    
    #define LIS3DH_CTRL_REG4                    0x23
    #define LIS3DH_CTRL_REG4_FSR_2G             0x80 // (±2g)
    #define LIS3DH_CTRL_REG4_FSR_4G             0x90 // (±4g)
    #define LIS3DH_CTRL_REG4_FSR_8G             0xA0 // (±8g)
    #define LIS3DH_CTRL_REG4_FSR_16G            0xB0 // (±16g)
    
    #define LIS3DH_CTRL_REG1                    0x20
    #define LIS3DH_CTRL_REG1_SF_1HZ             0x17 // (1Hz)
    #define LIS3DH_CTRL_REG1_SF_10HZ            0x27 // (10Hz)
    #define LIS3DH_CTRL_REG1_SF_25HZ            0x37 // (25Hz)
    #define LIS3DH_CTRL_REG1_SF_50HZ            0x47 // (50Hz)

 
#endif