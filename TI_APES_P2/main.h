/*****************************************************
 * File: main.h
 * Authors: Virag Gada and Vihanga Bare
 * Description: Header file for APES Project 2
 ****************************************************/

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "drivers/pinout.h"
#include "utils/uartstdio.h"

// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/systick.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"

#include "enet_lwip.h"


/* Macro to enable and disable unit testing */
//#define UNIT_TEST

//uint32_t system_clock_rate_hz;
uint32_t g_ui32SysClock;

// System clock rate, 120 MHz
#define SYSTEM_CLOCK    120000000U

typedef enum loglevel
{
  INFO,
  WARNING,
  ALERT,
  HEART_BEAT,
  INITIALIZATION
}LogLevel;

typedef enum{
  LOG_DATA,
  HEARTBEAT,
  DECIDE,
  SYSTEM_SHUTDOWN
}reqCmds;

typedef enum{
  MAIN_TASK,
  TEMP_TASK,
  PEDO_TASK,
  SOCKET_TASK,
  LOGGER_TASK,
  DECISION_TASK
}Sources;

typedef struct logger
{

  uint8_t sourceId;
  uint8_t requestID;
  uint8_t level;
  float data;
  char timestamp[32];
  char payload[100];
}LogMsg;

#define MAXLEN 100
#define QLEN   50

int exitflag;

#endif /* MAIN_H_ */
