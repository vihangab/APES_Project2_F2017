/*************************************************************************
* Authors : Virag Gada, Vihanga Bare
* Date : 11/1/2017
*
* File : logger.c
* Description : Source file for logger functions
*               -logger_state create_log_file(FILE **fptr, char *fileName)
*               -logger_state log_item(FILE *fptr, LogMsg * logStruct)
*               -logger_state log_string(FILE *fptr, uint8_t * data_TX)
*               -logger_state log_integer(FILE *fptr,uint32_t val)
*               -logger_state log_float(FILE *fptr,double val)
***************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "logger.h"

logger_state create_log_struct(LogMsg ** str){
  if((*str = (LogMsg *)malloc(sizeof(LogMsg)))==NULL){
    return NULL_PTR;
  }
  memset(*str,(uint8_t)'\0',sizeof(LogMsg));
  return DONE;
}

/* Function to create a log file */
logger_state create_log_file(FILE **fptr, char *fileName){
  if((*fptr=fopen(fileName,"w+"))==NULL){
    printf("%s\n","File create");
		return OPEN_ERROR;
	}
  return DONE;
}

/* Function to log a log structure */
logger_state log_item(FILE *fptr, LogMsg * logStruct){
	if((fptr == NULL)){
    printf("%s\n","NULL PTR 1");
  	return NULL_PTR;
	}
  if((logStruct == NULL)){
    printf("%s\n","NULL PTR 10");
  	return NULL_PTR;
	}

	char COLOR[10] = {(uint8_t)'\0'};
	switch(logStruct->level){
		case INFO: strcpy(COLOR,ANSI_COLOR_GREEN);
		           break;
		case WARNING: strcpy(COLOR,ANSI_COLOR_YELLOW);
		           break;
		case ALERT: strcpy(COLOR,ANSI_COLOR_RED);
		           break;
		case HEART_BEAT: strcpy(COLOR,ANSI_COLOR_BLUE);
		           break;
		case INITIALIZATION: strcpy(COLOR,ANSI_COLOR_CYAN);
		           break;
		default : strcpy(COLOR,ANSI_COLOR_RESET);
		          break;
	}
        time_t timeVal = time(NULL);
  char stringToLog[300] = {(uint8_t)'\0'};
	sprintf(stringToLog,"%s [%s] %s Message- %s %s\n", COLOR,taskNames[logStruct->sourceId],ctime(&timeVal),logStruct->payload, ANSI_COLOR_RESET);
	printf("%s",stringToLog);
	return log_string(fptr,stringToLog);
}

/* This function is used to take a string and then write it to a log file */
logger_state log_string(FILE *fptr, char * data_TX){
	if(fptr == NULL){
    //printf("%s\n","NULL PTR 2");
		return NULL_PTR;
	}
  //printf("%s\n","Printing on file");
	size_t length=strlen(data_TX);
	if(fwrite(data_TX,length,1,fptr)<0){
    printf("%s\n","Writing error");
		return WRITE_ERROR;
	}
  return DONE;
}


/* This function is used to convert integer to ascii and then write it to a log file */
logger_state log_integer(FILE *fptr,uint32_t val){
	if(fptr == NULL){
		return NULL_PTR;
	}
  char storage[10]={(uint8_t)'\0'};
	/* Function to convert integer to ascii */
  sprintf(storage,"%d",val);
	if(fwrite(storage,strlen(storage),1,fptr)<0){
		return WRITE_ERROR;
	}
	return DONE;
}

/*This function is used to convert float to ascii and then write it to a log file*/
logger_state log_float(FILE *fptr,double val){
	if(fptr == NULL){
		return NULL_PTR;
	}
	char storage[15] = {(uint8_t)'\0'};
	/* Function to convert floating point value to ascii */
	sprintf(storage,"%0.3f",val);
	if(fwrite(storage,strlen(storage),1,fptr)<0){
		return WRITE_ERROR;
	}
	return DONE;
}
