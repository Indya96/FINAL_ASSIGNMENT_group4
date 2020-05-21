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

#include "InterruptRoutines.h"

#include "I2C_Interface.h"

#include "SPI_Interface.h"



#define LIS3DH_DEVICE_ADDRESS 0x18

#define CTRL_REG3 0x22

#define CTRL_REG3_INT_ENABLE 0x2

#define LIS3DH_OUT_X_L 0x28

#define FIFO_CTRL_REG 0x2E

#define FIFO_CTRL_REG_FIFO_MODE 0x40

#define FIFO_CTRL_REG_BYPASS_MODE 0x00

uint8_t ctrl_reg3;

uint8_t fifo_ctrl_reg;


int main(void)
{

    I2C_Peripheral_Start();
    UART_Debug_Start();
    ctrl_reg3 = CTRL_REG3_INT_ENABLE; // ctrl_reg3 I1_OVERRUN bit set to '1'

    ErrorCode error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                           CTRL_REG3,
                                           ctrl_reg3);


    fifo_ctrl_reg = FIFO_CTRL_REG_FIFO_MODE; // Enabling FIFO mode
    ErrorCode error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                           FIFO_CTRL_REG,
                                           fifo_ctrl_reg);
    CyGlobalIntEnable; /* Enable global interrupts. */
    isr_FIFO_StartEx(Custom_FIFO_ISR); /* FIFO interrupt enabled pointing to proper function adress. */
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
