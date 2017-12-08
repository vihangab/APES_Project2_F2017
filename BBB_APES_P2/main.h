/*****************************************************
 * File: main.h
 * Authors: Virag Gada and Vihanga Bare
 * Description: Header file for APES Project 2 on BBB
 ****************************************************/

#ifndef MAIN_H_
#define MAIN_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include "gpio.h"
#include "logger.h"

/*Macro definitions */
#define MAX_SEND_BUFFER 4096

/* Function declarations */
void *SocketThread(void *);
void *LoggerThread(void *);

pthread_t socketThread;
pthread_t loggerThread;

#endif /* MAIN_H_ */
