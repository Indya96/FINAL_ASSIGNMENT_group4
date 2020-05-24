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

#define LIS3DH_OUT_X_L 0x28

#define FIFO_CTRL_REG 0x2E

#define FIFO_CTRL_REG_FIFO_MODE 0x40

#define FIFO_CTRL_REG_BYPASS_MODE 0x00




CY_ISR(Custom_FIFO_ISR)
{
  //Reading data from FIFO and storing them in AccData array;
  ErrorCode error = I2C_Peripheral_ReadRegisterMulti(LIS3DH_DEVICE_ADDRESS,
                                            LIS3DH_OUT_X_L,
                                            FIFO_DATA_DIM,
                                              &AccData[0]);
  if (error==NO_ERROR)
  {
    //Bypass mode Activation in order to reset the FIFO
    fifo_ctrl_reg = FIFO_CTRL_REG_BYPASS_MODE;

    ErrorCode error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                           FIFO_CTRL_REG,
                                           fifo_ctrl_reg);
    if(error==NO_ERROR)
    {
      fifo_ctrl_reg = FIFO_CTRL_REG_FIFO_MODE; //  FIFO Mode re-activated once FIFO is reset

      ErrorCode error = I2C_Peripheral_WriteRegister(LIS3DH_DEVICE_ADDRESS,
                                             FIFO_CTRL_REG,
                                             fifo_ctrl_reg);
    }


  }

  FlagFifo=1;
}
/* [] END OF FILE */
