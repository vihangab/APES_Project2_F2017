/*****************************************************
 * File: temp102.h
 * Authors: Virag Gada and Vihanga Bare
 * Description: Header file for TMP 102 sensor
 ****************************************************/


#include <stdio.h>
#include <stdlib.h>

#ifndef TMP102_LIBRARY_H_
#define TMP102_LIBRARY_H_

/* Temp sensor I2C address */
#define TMP_SENSOR_ADDR      (0x48)

/* Peripheral addresses */
#define GPIO_PORTB_BASE      (0x40005000)  // GPIO Port B
#define I2C0_BASE            (0x40020000)  // I2C0

//P1,P0 bit in Pointer Register of TMP 102 to select registers
#define SELECT_TEMP          (0x00)
#define SELECT_CONFIG        (0x01)
#define SELECT_TLOW          (0x20)
#define SELECT_THIGH         (0x03)

/* Config register values */
#define ENABLE_SHUTDOWN      (0x01)
#define DISABLE_SHUTDOWN     (0x00)
#define COMPARATOR_MODE      (0x00)
#define INTERRUPT_MODE       (0x02)
#define POLARITY_ZERO        (0x00)
#define POLARITY_ONE         (0x04)
#define FAULTS_ONE           (0x00)
#define FAULTS_TWO           (0x08)
#define FAULTS_FOUR          (0x10)
#define FAULTS_SIX           (0x18)
#define ONE_SHOT             (0x80)
#define NORMAL_MODE          (0x00)
#define EXTENDED_MODE        (0x10)
#define CONVERSION_RATE_0_25 (0x00)
#define CONVERSION_RATE_1    (0x40)
#define CONVERSION_RATE_4    (0x80)
#define CONVERSION_RATE_8    (0xC0)

void setupTMP102();

void readTMP102(double *digitalTemp);

#endif /* TMP102_LIBRARY_H_ */
