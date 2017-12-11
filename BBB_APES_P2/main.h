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
#define QLog            "/logq"
#define QMain           "/mainq"
#define QDecide         "/decisionq"

/* Function declarations */
void *SocketThread(void * input);
void *LoggerThread(void *);
void *DecisionThread(void *);
void initialize_queue(char * qName, mqd_t *msgHandle);

/*synchronisation variables */
pthread_mutex_t decisionQ_mutex;
pthread_mutex_t logQ_mutex;
pthread_mutex_t mainQ_mutex;


/* Queue Handles */
mqd_t logger_queue_handle;
mqd_t decision_queue