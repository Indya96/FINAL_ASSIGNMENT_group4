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


uint8_t counter_timer= 0;   


CY_ISR(Custom_TIMER_ISR)
{
    Timer_ReadStatusRegister();         //restart the timer
    
    if(counter_timer<(SENSOR_DATA_DIM))
    {
        
        SensData[counter_timer]= ADC_DelSig_Read32();  //read data from ADC
        
        /* verify the value */
        if (SensData[counter_timer] < 0)        SensData[counter_timer] = 0;
        if (SensData[counter_timer] > 65535)    SensData[counter_timer] = 65535;
        
        SensBytes[2*counter_timer]= (uint8_t)(SensData[counter_timer] & 0xFF);  //LSB
        SensBytes[2*counter_timer +1]= (uint8_t)(SensData[counter_timer]>>8) ;  //MSB
        
        counter_timer ++;
    }
    
    else if(counter_timer == (SENSOR_DATA_DIM))
    {
        int i=0;
        for(i=0;i<SENSOR_DATA_DIM*2;i++)
        {
            SensBytes_old[i]=SensBytes[i];
        }
        counter_timer=0;
        SensorDataReady=1;
    }
       
    
    
    
}



/* [] END OF FILE */
