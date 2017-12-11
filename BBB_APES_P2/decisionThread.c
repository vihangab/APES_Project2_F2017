/*****************************************************
 * File: DecisionThread.c
 * Authors: Virag Gada and Vihanga Bare
 * Description: Source file for Decision task
 ****************************************************/

#include "main.h"
#include "logger.h"


struct mq_attr attr;

void *DecisionThread(void *args)
{
  uint32_t msgPriority;
  LogMsg *logmsg0;
  LogMsg logmsg00;
  uint32_t bytes_read,bytes_sent;
  //char command[MAX_SEND_BUFFER] = {(int)'\0'};
  //printf("Log file create status - %d,\n", create_log_file(&logFile, "dataLog"));

  if(create_log_struct(&logmsg0)!=DONE){
		printf("%s\n","Error creating struct logmsg0");
	}/*else if(create_log_struct(&logmsg00)!=DONE){
		printf("%s\n","Error creating struct logmsg00");
	}*/else {
    while(1){
      mq_getattr(decision_queue_handle, &attr);
      while(attr.mq_curmsgs > 0)
				{
					//pthread_mutex_lock(&decisionQ_mutex);
					bytes_read = mq_receive(decision_queue_handle, (char *)&logmsg0, sizeof(LogMsg)+1, &msgPriority);
          //bytes_read = mq_receive(decision_queue_handle, (char *)command,MAX_SEND_BUFFER, &msgPriority);
					if (bytes_read == -1)
					{
						perror("[DecisionThread] Failed to recieve:");
					}
					else
					{
						printf("[DecisionThread] Queue %s currently holds %ld messages\n",QLog,attr.mq_curmsgs);
						mq_getattr(decision_queue_handle, &attr);
            //printf("%s\n",command);
            //memset(command,(int)'\0',MAX_SEND_BUFFER);
						if(logmsg0->requestID == DECIDE)
						{
              if(logmsg0->sourceId == TEMP_TASK){
                if(logmsg0->data>TEMPERATURE_THRESHOLD){ // If value greater than threshold then send send error log
                  memset(&logmsg00,(int)'\0',sizeof(LogMsg));
                  logmsg00.sourceId = DECISION_TASK;
                  logmsg00.requestID = LOG_DATA;
                  logmsg00.level = ALERT;
                  logmsg00.timestamp = time(NULL);
                  sprintf(logmsg00.payload,"Body temperture %.2f higher than normal",logmsg0->data);
                  printf("%s\n", logmsg00.payload);
                  //pthread_mutex_lock(&logQ_mutex);
                  if ((bytes_sent = mq_send (logger_queue_handle,(const char*)&logmsg00, sizeof(LogMsg), 1)) != 0)
                  {
                    perror ("[DecisionThread] Sending Decision");
                  }
                  //pthread_mutex_unlock(&logQ_mutex);
                  printf("%s\n","Temperature greater than normal");
                }
              }

					  }// Clear buffer

					}
					//pthread_mutex_unlock(&decisionQ_mutex);
          memset(logmsg0,(int)'\0',sizeof(LogMsg));
				}
    }
  }
  mq_close(decision_queue_handle);
  pthread_exit(NULL);
}
