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
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdbool.h>
#include "gpio.h"
#include "logger.h"

/*Macro definitions */
#define MAX_SEND_BUFFER        (1024)
#define TEMPERATURE_THRESHOLD  (28)
#define QLog                   "/logq"
#define QMain                  "/mainq"
#define QDecide                "/decisionq"
#define DEVICE_NAME	       "/dev/SimpleCharDrv" 
#define QLEN 		       32

int32_t fd; //file descripter for led device

typedef struct data
{
	bool state;
	long period;
	long duty;
}data_t;

data_t valtodriver;

sig_atomic_t SIGINT_EVENT;

/* Function declarations */
void *SocketThread(void * input);
void *LoggerThread(void *);
void *DecisionThread(void *);
void create_interval_timer(float timer_val);
void initialize_queue(char * qName, mqd_t *msgHandle);
void sighandler_sigint(int signum);

/*synchronisation variables */
pthread_mutex_t logQ_mutex;
pthread_mutex_t mainQ_mutex;
pthread_mutex_t decisionQ_mutex;
pthread_cond_t condvar;

/* Queue Handles */
mqd_t logger_queue_handle;
mqd_t decision_queue_handle;
mqd_t main_queue_handle;

/* Thread ID's */
pthread_t socketThread;
pthread_t loggerThread;
pthread_t decisionThread;

#endif /* MAIN_H_ */
