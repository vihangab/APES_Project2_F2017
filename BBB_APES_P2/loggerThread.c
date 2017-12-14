/*****************************************************
 * File: loggerThread.c
 * Authors: Virag Gada and Vihanga Bare
 * Description: Source file for logger task
 ****************************************************/

#include "main.h"
#include "logger.h"
#define BUFFER_SIZE 80

struct mq_attr attr;
FILE *logFile;

void *LoggerThread(void *args)
{
  uint32_t msgPriority;
  LogMsg *logmsg1;
  uint32_t bytes_read;
  char command[BUFFER_SIZE] = {(int)'\0'};
  printf("Log file create status - %d,\n", create_log_file(&logFile, "dataLog"));
  if(create_log_struct(&logmsg1)!=DONE){
		printf("%s\n","Error creating struct");
	}else{
    while(1){
      //mq_getattr(logger_queue_handle, &attr);
      //while(attr.mq_curmsgs > 0)
				//{
					//pthread_mutex_lock(&logQ_mutex);
          //sleep(2);
					bytes_read = mq_receive(logger_queue_handle, (char *)&logmsg1, sizeof(LogMsg), &msgPriority);
          //bytes_read = mq_receive(logger_queue_handle, (char *)&command,BUFFER_SIZE , &msgPriority);
					if (bytes_read == -1)
					{
						perror("[LoggerThread] Failed to recieve:");
					}
					else
					{
						if(logmsg1->requestID == LOG_DATA)
						{
							//printf("Logging status -%d\n",log_item(logFile,logmsg1) );
              log_item(logFile,logmsg1);
							//printf ("[LoggerThread] Source ID: %d \n", logmsg1->sourceId);
							//printf ("[LoggerThread] Log Level: %d \n", logmsg1->level);
							printf ("[LoggerThread] Payload: %s \n", logmsg1->payload);
							//printf ("[LoggerThread] Timestamp: %s \n", logmsg1->timestamp);
					  }else if(logmsg1->requestID == SYSTEM_SHUTDOWN){
              printf("%s\n","Closing logger");
              fclose(logFile);
              break;
            }
		//printf("command received = %s",command);
		// Clear buffer
            mq_getattr(logger_queue_handle, &attr);
            //printf("[LoggerThread] Queue %s currently holds %ld messages\n",QLog,attr.mq_curmsgs);
        	}
					//pthread_mutex_unlock(&logQ_mutex);
          //memset(logmsg1,(int)'\0',sizeof(LogMsg));
				//}
    }
  }
  printf("%s\n","Exiting logger");
  free(logmsg1);
  pthread_exit(NULL);
}
