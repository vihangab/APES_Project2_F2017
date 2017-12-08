/*****************************************************
Name - led.h
Author - Virag Gada and Vihanga Bare
Description - Header file for our LED library
*****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static char led_path[] = {"/sys/class/leds/beaglebone:green:usr0/brightness"};

typedef enum {
  SET,
  RESET,
  ERR_OPEN,
  ERR_WRITE,
  ERR_READ,
  ERR_CLOSE
}gpioState;

gpioState pinSet(char * devAddr);

gpioState pinReset(char * devAddr);
