/*****************************************************
 * File: temp102.h
 * Authors: Virag Gada and Vihanga Bare
 * Description: Header file for TMP 102 sensor
 ****************************************************/


#include <tmp102.h>
#include "main.h"

void setupTMP102(){

    // Enable GPIOB peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    // Configure pin muxing
    // Set PB2 to SCL, PB3 to SDA
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    // Select I2C function for these pins
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    // Enable I2C0 peripheral
    SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0));


    /* Enable and initialize I2C0 Master module
     * data transfer rate 400kbps */
    I2CMasterInitExpClk(I2C0_BASE, system_clock_rate_hz, true);

    UARTprintf("Temp sensor setup done\n");
}

void readTMP102(double *digitalTemp){

    uint8_t reg_value[2];
    int16_t intTemp;

    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CMasterSlaveAddrSet(I2C0_BASE, TMP_SENSOR_ADDR, false); // false means set master to write

    // Place the data to be sent in the data register
    I2CMasterDataPut(I2C0_BASE, SELECT_TEMP);

    // Initiate send from master
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    // Wait until master module is done transferring.
    while(I2CMasterBusy(I2C0_BASE));

    // true means set master to read
    I2CMasterSlaveAddrSet(I2C0_BASE, TMP_SENSOR_ADDR, true);

    // Tell the master to read data.
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

    // receive first byte
    while(I2CMasterBusy(I2C0_BASE));
    reg_value[0] = I2CMasterDataGet(I2C0_BASE);
    //UARTprintf("reg[0] - 0x%x\n",reg_value[0]);

    // receive second byte
    while(I2CMasterBusy(I2C0_BASE));
    reg_value[1] = I2CMasterDataGet(I2C0_BASE);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    //UARTprintf("reg[1] - 0x%x\n",reg_value[1]);

    /* Check resolution and find value */
    if(reg_value[1]&0x01){//13 bit mode
      intTemp = (reg_value[0]<<5)|(reg_value[1]>>3);
      /* Take 2's compliment if negative*/
      if(intTemp > 0xFFF){
        intTemp |= 0xE000;
      }
    }else{ // 12 bit mode
      intTemp = (reg_value[0]<<4)|(reg_value[1]>>4);
      /* Take 2's compliment if negative*/
      if(intTemp > 0x7FF){
          intTemp |= 0xF000;
      }
    }
    UARTprintf("integer value - %d\n",intTemp);

    *digitalTemp = 0.0625*intTemp;

}
