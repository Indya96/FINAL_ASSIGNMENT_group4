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
#include "stdio.h"
#include "Pages.h"
#include "25LC256.h"
#include "Registers.h"


uint8_t   Out_Data[DATA_PACKET_DIM];

  //Number of bytes to be written during page writing and pointer to data to be written.
uint8_t Bytes_for_page= 64;
uint8_t * data_pointer;


  //Flags Flow
StartAcquisition_flag = 0;
StartStream_flag      = 0;




uint8_t ctrl_reg3;

uint8_t fifo_ctrl_reg;

uint16_t EEPROM_Adress;

volatile uint8_t SensorDataReady = 0;

int main(void)
{
    SPIM_1_Start();
    I2C_Peripheral_Start();
    UART_Debug_Start();
    SPIM_1_Start();
    CyDelay(10);

    ADC_DelSig_start();
    TIMER_start();

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
    isr_TIMER_StartEx(custom_TIMER_ISR);   /* TIMER interrupt enabled pointing to proper function adress. */

    uint8_t Packets[192];
    uint8_t index=0;

    uint8_t OutputData[DATA_PACKET_DIM];

    uint16_t  EEPROM_read_address= INITIAL_ADDRESS;

    //uint16_t EEPROM_index;   //non penso che serva

    int16_t Z_acc_digit;
    int16_t Y_acc_digit;
    int16_t X_acc_digit;

    int16_t Z_acc_mg;
    int16_t Y_acc_mg;
    int16_t X_acc_mg;

    int16 sensitivity;

    //sensitivity = 4 if FSR +-2g
    //sensitivity = 8 if FSR +-4g
    //sensitivity = 16 if FSR +-8g
    //sensitivity = 32 if FSR +- 16g


    uint8_t UART_OutArray[UART_PACKET_DIM];

    UART_OutArray[0]= 0xA0;                     //header
    UART_OutArray[UART_PACKET_DIM - 1]= 0xC0;   //footer



    for(;;)
    {
      /*******************************************************************************************************************/
/*                                                 ACQUISITION                                                     */
/*******************************************************************************************************************/

        if(StartAcquisition_flag==1){

          if(FlagFifo==1 && SensorDataReady == 1){

              index=0;

              for(i=0; i<32; i++){

                  Packets[index] = *(AccData+i*6+4)>>6 | *(AccData+i*6+5)<<2;
                  index++;
                  Packets[index] = (*(AccData+i*6+5)>>6) | ((*(AccData+i*6+2)&0xC0)>>4) | ((*(AccData+i*6+3)&0x0F)<<4);
                  index++;
                  Packets[index] =((*(AccData+i*6+3)&0xF0)>>4) | (*(AccData+i*6)>>2) | (*(AccData+i*6+1)<<6);
                  index++;
                  Packets[index] =  AccData[i*6+1]>>2;
                  index++;
                  Packets[index] =  SensBytes_old[i*2];
                  index++;
                  Packets[index] =  SensBytes_old[i*2+1];
                  index++;

              }

              /*I Check the adress where i want to start writing*/
              EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
              EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
              EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);

              /*I have to write 192 Bytes = 3*64 Bytes = 3 Pages */
              if (EEPROM_Adress < EEPROM_DIMENSION){

                  for(uint16_t t=0;t<3;t++){

                      Adress_to_write = EEPROM_Adress + t*64;
                      data_pointer = &Packets[t*64];
                      EEPROM_writePage(Adress_to_write, data_pointer, Bytes_for_page);
                      EEPROM_waitForWriteComplete();
                  }

                  EEPROM_Adress           = Adress_to_write + Bytes_for_page;
                  EEPROM_Adress_Pointer_L = (uint8_t) (EEPROM_Adress & 0xFF);
                  EEPROM_Adress_Pointer_H = (uint8_t) ((EEPROM_Adress & 0xFF00)>>8);

                  EEPROM_writeByte(0x0000, EEPROM_Adress_Pointer_L);
                  EEPROM_waitForWriteComplete();
                  EEPROM_writeByte(0x0001, EEPROM_Adress_Pointer_H);
                  EEPROM_waitForWriteComplete();
              }

              else{
                  full_or_empty = 1;
                  LED_EEPROM_FULL_TOGGLE();
              }

              Timer_sensor_ReadStatusRegister();         //restart the time
              counter_timer=0;
              FlagFifo=0;
              SensorDataReady=0;

          }
        }
        
        /*******************************************************************************************************************/
        /*                                                  STREAMING                                                      */
        /*******************************************************************************************************************/
        
        if(StartStream_flag==1){
            
            EEPROM_Adress_Pointer_L = EEPROM_readByte(0x0000);
            EEPROM_Adress_Pointer_H = EEPROM_readByte(0x0001);
            EEPROM_Adress = (uint16_t) EEPROM_Adress_Pointer_L | (EEPROM_Adress_Pointer_H<<8);

            Adress_to_read = EEPROM_INITIAL_ADDRESS;

            while (Adress_to_read < EEPROM_Adress && StartStream_flag){
                
                UART_ClearTxBuffer(); // Clean Tx Buffer
              
                EEPROM_readPage(Adress_to_read, Out_Data, DATA_PACKET_DIM);
              
                xl = (uint8_t)  (Out_Data[2]&0x30)<<2;
                xh = (uint8_t)  (Out_Data[2]&0xC0)>>6  |  (Out_Data[3]&0x3f)<<2;
                yl = (uint8_t)  (Out_Data[1]&0x0C)<<4;
                yh = (uint8_t) ((Out_Data[1]&0xF0)>>4) | ((Out_Data[2]&0x0F)<<4);                      
                zl = (uint8_t)  (Out_Data[0]&0x03)<<6;
                zh = (uint8_t) ((Out_Data[0]&0xFC)>>2) |  (Out_Data[1]&0x03)<<6;
                
                x = (int16)((xh<<8)|(xl))>>6;               //X-axis acceleration in digit
                y = (int16)((yh <<8)|(yl))>>6;              //Y-axis acceleration in digit
                z = (int16)((zh <<8)|zl)>>6;                //Z-axis acceleration in digit
                
                z_uart = (int16_t) (((float32)z)*Sensitivity*0.981 + 0.5);
                y_uart = (int16_t) (((float32)y)*Sensitivity*0.981 + 0.5);
                x_uart = (int16_t) (((float32)x)*Sensitivity*0.981 + 0.5);
 
                UART_OutArray[1] = (uint8_t)(z_uart & 0xFF);     //LSB Z
                UART_OutArray[2] = (uint8_t)(z_uart >>8);        //MSB Z
                UART_OutArray[3] = (uint8_t)(y_uart & 0xFF);     //LSB Y
                UART_OutArray[4] = (uint8_t)(y_uart >>8);        //MSB Y
                UART_OutArray[5] = (uint8_t)(x_uart & 0xFF);     //LSB Z
                UART_OutArray[6] = (uint8_t)(x_uart >>8);        //MSB Z
                UART_OutArray[7] = Out_Data[4];                   //LSB of sensor
                UART_OutArray[8] = Out_Data[5];                   //MSB of sensor
            
                UART_PutArray(UART_OutArray, UART_PACKET_DIM);   
                CyDelay(10);                                // 10ms
                
                Adress_to_read=Adress_to_read +6;

                if(Adress_to_read >= EEPROM_Adress){
                    Adress_to_read = EEPROM_INITIAL_ADDRESS;       
                }
            }

        }

    }

}

/* [] END OF FILE */
