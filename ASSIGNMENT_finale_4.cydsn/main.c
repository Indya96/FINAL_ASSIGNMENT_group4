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

#define EEPROM_INITIAL_ADDRESS 0x80

#define EEPROM_PAGE_DIM 64



uint8_t ctrl_reg3;

uint8_t fifo_ctrl_reg;

uint16_t EEPROM_Adress;

int main(void)
{
    SPIM_1_Start();
    I2C_Peripheral_Start();
    UART_Debug_Start();
    CyDelay(10);

    uint8_t EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
    uint8_t EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
    uint16_t EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8)
    if(EEPROM_Adress<EEPROM_INITIAL_ADDRESS)
    {
      EEPROM_writeByte(0x0000, (uint8_t) EEPROM_INITIAL_ADDRESS);
      EEPROM_waitForWriteComplete();
      // EEPROM_writeByte(0x0001,(uint8_t) 0);
    }

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

    uint8_t Packets[192];
    uint8_t index=0;

    for(;;)
    {
      /* Sending data to EEPROM*/
      if(FlagFifo)
        {
          if(SensorDataReady)
          {
              for(uint8_t i=0;i<32;i++)
              {
                Packets[index]=(AccData[i*6+4]>>6) | (AccData[i*6+5]<<2);
                index++;
                Packets[index] = (AccData[i*6+5]>>6) | (acc[i*6+2]>>4) | (acc[i*6+3]<<4);
                index++;
                Packets[index]= (AccData[i*6+3]>>4) | (AccData[i*6]>>6) | (AccData[i*6+1]<<6)  ;
                index++;
                Packets[index]=AccData[i*6+1]>>2;
                index++;
                Packets[index]=SensBytes_old[i*2];
                index++;
                Packets[index]=SensBytes_old[i*2+1];
                index++;
              }
              /*I Check the adress where i want to start writing*/
               EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
               EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
               EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8)
              /*I have to write 192 Bytes = 3*64 Bytes = 3 Pages */
              if (EEPROM_Adress!=0x7FFF)
              {
                  for(uint8_t t=0;t<3;t++)
                  {
                    EEPROM_writePage(EEPROM_Adress+(t*EEPROM_PAGE_DIM), (uint8_t*) Packets+t*EEPROM_PAGE_DIM, EEPROM_PAGE_DIM);
                    EEPROM_waitForWriteComplete();
                  }
                  EEPROM_Adress = EEPROM_Adress + EEPROM_PAGE_DIM*3;
                  EEPROM_Adress_Pointer_L = (uint8_t) (EEPROM_Adress & 0xFF);
                  EEPROM_Adress_Pointer_H = (uint8_t) (EEPROM_Adress & 0xFF00)>>8;
                  EEPROM_writeByte(0x0000, (uint8_t) EEPROM_Adress_Pointer_L);
                  EEPROM_waitForWriteComplete();
                  EEPROM_writeByte(0x0001, (uint8_t) EEPROM_Adress_Pointer_H);
                  EEPROM_waitForWriteComplete();
              }
              else
              {
                //Blinking Led
              }

          }

        }
    }
}

/* [] END OF FILE */
