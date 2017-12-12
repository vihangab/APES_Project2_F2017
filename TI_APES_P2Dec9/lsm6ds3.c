/*****************************************************
 * File: lsm6ds3.h
 * Authors: Virag Gada and Vihanga Bare
 * Description: Source file for LSM6DS3 sensor
 ****************************************************/

#include <lsm6ds3.h>
#include "main.h"

void setupI2C2(){
    // Enable GPIOL peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));

    // Configure pin muxing
    // Set PL1 to SCL, PL0 to SDA
    GPIOPinConfigure(GPIO_PL1_I2C2SCL);
    GPIOPinConfigure(GPIO_PL0_I2C2SDA);

    // Select I2C function for these pins
    GPIOPinTypeI2CSCL(GPIO_PORTL_BASE, GPIO_PIN_1);
    GPIOPinTypeI2C(GPIO_PORTL_BASE, GPIO_PIN_0);

    // Enable I2C2 peripheral
    SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C2);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C2));

    /* Enable and initialize I2C2 Master module
     * data transfer rate 400kbps */
    I2CMasterInitExpClk(I2C2_BASE, system_clock_rate_hz, true);

    UARTprintf("I2C2 setup");
}

void setupLSM6DS3(){

  setupI2C2();

  //Enable the XYZ axes of accelerometer
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false); // Tell the master module what address it will place on the bus when
                                                                // communicating with the slave.
                                                                // false means set master to write
  I2CMasterDataPut(I2C2_BASE, CTRL9_XL_ADDR);                   // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START); // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterDataPut(I2C2_BASE, ENABLE_XYZ_AXES);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  SysCtlDelay(100); // 1us delay

  // Turn on accelerometer, ODR_XL = 26Hz, FS_XL = 2g
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false);
  I2CMasterDataPut(I2C2_BASE, CTRL1_XL_ADDR);                   // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START); // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterDataPut(I2C2_BASE, ENABLE_ACCELEROMETER);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  SysCtlDelay(100); // 1us delay

  // Enable Embedded Functions
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false);
  I2CMasterDataPut(I2C2_BASE, CTRL10_C_ADDR);                   // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START); // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterDataPut(I2C2_BASE, ENABLE_EMBEDDED_FUNC);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  SysCtlDelay(100); // 1us delay

  // Enable Pedometer Algorithm
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false);
  I2CMasterDataPut(I2C2_BASE, TAP_CFG);                         // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START); // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterDataPut(I2C2_BASE, ENABLE_PEDOMETER_ALGO);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  SysCtlDelay(100); // 1us delay

  // Send step interrupt to drive INT1 pin
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false);
  I2CMasterDataPut(I2C2_BASE, INT1_CTRL);                       // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START); // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterDataPut(I2C2_BASE, ENABLE_STEP_INTERRUPT);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  SysCtlDelay(100); // 1us delay

  UARTprintf("Pedometer sensor setup done\n");

  /* Read status register
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false);
  I2CMasterDataPut(I2C2_BASE, STATUS_REG_ADDR);                   // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND);      // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, true);  // true means set master to read
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  uint8_t status = I2CMasterDataGet(I2C2_BASE);
  while(I2CMasterBusy(I2C2_BASE));
  SysCtlDelay(100); // 1us delay

  UARTprintf("Sensor status - %d\n", status);*/
}

void readStepCount(uint16_t *stepCount){
  uint8_t reg_value[2];
  //int16_t stepCount;

  // Read status register
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false);
  I2CMasterDataPut(I2C2_BASE, STATUS_REG_ADDR);                   // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND);      // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, true);  // true means set master to read
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  uint8_t status = I2CMasterDataGet(I2C2_BASE);
  while(I2CMasterBusy(I2C2_BASE));
  SysCtlDelay(100); // 1us delay

  UARTprintf("Sensor status - %d\n", status);

  // Read value from the step counter l register
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false);
  I2CMasterDataPut(I2C2_BASE, STEP_COUNTERL);                   // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND);      // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, true);  // true means set master to read
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  reg_value[0] = I2CMasterDataGet(I2C2_BASE);
  while(I2CMasterBusy(I2C2_BASE));
  SysCtlDelay(100); // 1us delay

  //UARTprintf("Low Step value - %d\n",reg_value[0]);

  // Read value from the step counter h register
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, false);
  I2CMasterDataPut(I2C2_BASE, STEP_COUNTERH);                   // Place the data to be sent in the data register
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND);      // Initiate send from master
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  I2CMasterSlaveAddrSet(I2C2_BASE, LSM6DS3_SENSOR_ADDR, true);  // true means set master to read
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
  while(I2CMasterBusy(I2C2_BASE));                              // Wait until master module is done transferring.
  reg_value[1] = I2CMasterDataGet(I2C2_BASE);
  while(I2CMasterBusy(I2C2_BASE));
  SysCtlDelay(100); // 1us delay

  //UARTprintf("High Step value - %d\n",reg_value[1]);


  *stepCount = (uint16_t)reg_value[1]<<8 | reg_value[0];

  //UARTprintf("Step count in function- %d\n",*stepCount);
}
