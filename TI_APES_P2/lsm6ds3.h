/*****************************************************
 * File: lsm6ds3.h
 * Authors: Virag Gada and Vihanga Bare
 * Description: Header file for LSM6DS3 sensor
 ****************************************************/

#ifndef LSM6DS3_H_
#define LSM6DS3_H_

#include <stdint.h>

/* Pedometer sensor I2C address */
#define LSM6DS3_SENSOR_ADDR    (0x6B)

/* Peripheral addresses */
#define GPIO_PORTL_BASE        (0x40062000)  // GPIO Port L
#define I2C2_BASE              (0x40022000)  // I2C2


/* Register addresses */
#define CTRL9_XL_ADDR          (0x18)

#define CTRL1_XL_ADDR          (0x10)
#define CTRL10_C_ADDR          (0x19)
#define TAP_CFG                (0x58)
#define INT1_CTRL              (0x0D)
#define STEP_COUNTERL          (0x4B)
#define STEP_COUNTERH          (0x4C)
#define STATUS_REG_ADDR        (0x1E)

/* Values for Acceleromerter */
#define ENABLE_XYZ_AXES        (0x38)


/* Values for Pedometer function */
#define ENABLE_ACCELEROMETER   (0x20) // Turn on Accelerometer, CTRL1_XL
                                      // ODR_XL = 26Hz, FS_XL = 2g
#define ENABLE_EMBEDDED_FUNC   (0x3C) // Enable Embedded Functions, CTRL10_C
#define ENABLE_PEDOMETER_ALGO  (0x40) // Enable Pedometer Algorithm, TAP_CFG
#define ENABLE_STEP_INTERRUPT  (0x80) // Step detector interrupt drive to INT1 pin

void setupI2C2();

void setupLSM6DS3();

void readStepCount(uint16_t *stepCount);

#endif /* LSM6DS3_H_ */
