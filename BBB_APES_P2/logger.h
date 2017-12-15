/*************************************************************************
* Authors : Virag Gada, Vihanga Bare
* Date : 11/1/2017
*
* File : logger.h
* Description : Header file for logger functions
*               -logger_state create_log_file(FILE **fptr, char *fileName)
*               -logger_state log_item(FILE *fptr, LogMsg * logStruct)
*               -logger_state log_string(FILE *fptr, uint8_t * data_TX)
*               -logger_state log_integer(FILE *fptr,uint32_t val)
*               -logger_state log_float(FILE *fptr,double val)
***************************************************************************/
#ifndef LOGGER_H_
#define LOGGER_H_

#define ANSI_COLOR_RED     ("\x1b[31m")
#define ANSI_COLOR_GREEN   ("\x1b[32m")
#define ANSI_COLOR_YELLOW  ("\x1b[33m")
#define ANSI_COLOR_BLUE    ("\x1b[34m")
#define ANSI_COLOR_MAGENTA ("\x1b[35m")
#define ANSI_COLOR_CYAN    ("\x1b[36m")
#define ANSI_COLOR_RESET   ("\x1b[0m")

static char *taskNames[]={"MainThread","TempThread","PedoThread","SocketThread","LoggerThread","DecisionThread"};

typedef enum
{
  DONE,
  NULL_PTR,
  OPEN_ERROR,
  WRITE_ERROR
}logger_state;

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

logger_state create_log_struct(LogMsg ** str);

/****************************************************************************
* Function to create a log file and return the file descriptor in input pointer
* FILE **fptr - Pointer to the address of file descriptor
* char * fileName - Pointer to the file name string
* logger_state(Return) - Return the state of log operation
****************************************************************************/
logger_state create_log_file(FILE **fptr, char *fileName);

/****************************************************************************
* Function to log the log structure
* FILE *fptr - Pointer to the file descriptor
* LogMsg * logStruct - Pointer to the log structure
* logger_state(Return) - Return the state of log operation
****************************************************************************/
logger_state log_item(FILE *fptr, LogMsg * logStruct);

/****************************************************************************
* This function is used to take a string and then write it to a log file
* FILE *fptr - Pointer to the file descriptor to write to
* char data_TX - Pointer to the string to be logged
* logger_state(Return) - Return the state of log operation
****************************************************************************/
logger_state log_string(FILE *fptr, char * data_TX);

/****************************************************************************
* This function is used to convert integer to ascii and then write it to a
* log file
* FILE *fptr - Pointer to the file descriptor to write to
* uint32_t val - Integer value to be logged
* logger_state(Return) - Return the state of log operation
****************************************************************************/
logger_state log_integer(FILE *fptr,uint32_t val);

/****************************************************************************
* This function is used to convert float to ascii and then write it to a
* log file
* FILE *fptr - Pointer to the file descriptor to write to
* double val - Floating point value to be logged
* logger_state(Return) - Return the state of log operation
****************************************************************************/
logger_state log_float(FILE *fptr,double val);

#endif /* LOGGER_H_ */
