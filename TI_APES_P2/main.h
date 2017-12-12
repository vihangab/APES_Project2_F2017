/*****************************************************
 * File: main.h
 * Authors: Virag Gada and Vihanga Bare
 * Description: Header file for APES Project 2
 ****************************************************/

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include "drivers/pinout.h"
#include "utils/uartstdio.h"

// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"


uint32_t system_clock_rate_hz;

// System clock rate, 120 MHz
#define SYSTEM_CLOCK    120000000U


#endif /* MAIN_H_ */
