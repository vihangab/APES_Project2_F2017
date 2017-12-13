/*****************************************************
 * File: main.c
 * Authors: Virag Gada and Vihanga Bare
 * Description: Source file for APES Project 2 on BBB
 ****************************************************/

#include "main.h"

void sighandler_sigint(int signum)
{
	printf("Caught signal sigint, coming out...\n");
  SIGINT_EVENT = 1;	//set flag for SIGINT event
	LogMsg logmsg1;
  uint32_t bytes_sent;
  logmsg1.sourceId = MAIN_TASK;
  logmsg1.level = ALERT;
  logmsg1.requestID = SYSTEM_SHUTDOWN;
  logmsg1.timestamp = time(NULL);
  strcpy(logmsg1.payload,"Shutting down");
  logmsg1.data = 0;
  /* Send to all tasks queues*/
  if ((bytes_sent = mq_send (decision_queue_handle,(const char*)&logmsg1, sizeof(LogMsg), 2)) != 0) //can be changed later to light queue handle
  {
    perror ("[MainThread] Sending to Decision");
  }
  printf("%s\n","Send to all queues");
  sleep(0.5);
  if ((bytes_sent = mq_send (logger_queue_handle,(const char*)&logmsg1, sizeof(LogMsg), 2)) != 0) //can be changed later to light queue handle
  {
    perror ("[MainThread] Sending to Log");
  }
  printf("%s\n","Sent to all queues");
}

int main()
{
  int32_t retval = 0;
  int32_t alive;
  int32_t bytes_sent;
  char threadName[20];
  /* Initialize a signal handler for SIGINT */
  signal(SIGINT, sighandler_sigint);
  SIGINT_EVENT = 0;
  LogMsg *logmsg0;

  /* Initialize all locks */
  pthread_mutex_init(&decisionQ_mutex,NULL);
  pthread_mutex_init(&logQ_mutex,NULL);
  pthread_mutex_init(&mainQ_mutex,NULL);

  pthread_cond_init(&condvar,NULL);

  /* Initialize all the queues */
  initialize_queue(QLog,&logger_queue_handle);
  initialize_queue(QDecide,&decision_queue_handle);
  //initialize_queue(QMain,&main_queue_handle);

  retval = pthread_create(&loggerThread,NULL,&LoggerThread,NULL);
	if(retval != 0)
	{
		printf("Thread Creation failed, error code - %d\n", retval);
		pinSet(led_path);
	}
  sleep(1);

	retval = pthread_create(&socketThread,NULL,&SocketThread,NULL);
	if(retval != 0)
	{
		printf("Thread Creation failed, error code - %d\n", retval);
		pinSet(led_path);
	}
  sleep(1);

  retval = pthread_create(&decisionThread,NULL,&DecisionThread,NULL);
  if(retval != 0)
  {
    printf("Thread Creation failed, error code - %d\n", retval);
    pinSet(led_path);
  }

  create_interval_timer(2);
  if(create_log_struct(&logmsg0)!=DONE){
    printf("%s\n","Error creating struct");
  }else{
  	while(1){
      pthread_mutex_lock(&mainQ_mutex);
      //keep waiting for timer signal
      pthread_cond_wait(&condvar,&mainQ_mutex);
      pthread_mutex_unlock(&mainQ_mutex);

      if(SIGINT_EVENT){
        printf("%s\n","Exiting Main");
        break;
      }
      if((alive = pthread_kill(socketThread, 0))==3) //Check with ERSCH flag
      {
        printf("%s\n","Socket Thread Dead");
        strcpy(threadName,taskNames[1]);
        pinSet(led_path);
      }else if((alive = pthread_kill(loggerThread, 0))==3)
      {
        printf("%s\n","Logger Thread Dead");
        strcpy(threadName,taskNames[2]);
        pinSet(led_path);
      }else if((alive = pthread_kill(decisionThread, 0))==3)
      {
        printf("%s\n","Decision Thread Dead");
        strcpy(threadName,taskNames[3]);
        pinSet(led_path);
      }
      if(alive == 3){
        logmsg0->sourceId = MAIN_TASK;
        logmsg0->level = ALERT;
        logmsg0->requestID = LOG_DATA;
        logmsg0->timestamp = time(NULL);
        sprintf(logmsg0->payload,"%s thread dead",threadName);
        //pthread_mutex_lock(&logQ_mutex);
        if ((bytes_sent = mq_send (logger_queue_handle,(const char*)&logmsg0, sizeof(LogMsg), 2)) != 0) //can be changed later to light queue handle
        {
          perror ("[MainThread] Sending:");
        }
        //pthread_mutex_unlock(&logQ_mutex);
      }else{
        printf("%s\n","[MainThread] All tasks alive");
      }
  	}
  }
  free(logmsg0);

  printf("%s\n","Main while ends");
	retval = pthread_join(decisionThread,NULL);
	if(retval != 0)
	{
		printf("pthread join failed, error code - %d\n", retval);
	}
  printf("%s\n","Decision nikala");

	retval = pthread_join(loggerThread,NULL);
	if(retval != 0)
	{
		printf("pthread join failed, error code - %d\n", retval);
	}
  printf("%s\n","Logger nikala");

	retval = pthread_join(socketThread,NULL);
	if(retval != 0)
	{
		printf("pthread join failed, error code - %d\n", retval);
	}
  printf("%s\n","Sab nikla nikala");
	retval = pthread_mutex_destroy(&logQ_mutex);
	if(retval != 0)
	{
		printf("mutex destroy failed, error code - %d\n", retval);
	}

	retval = pthread_mutex_destroy(&mainQ_mutex);
	if(retval != 0)
	{
		printf("mutex destroy failed, error code - %d\n", retval);
	}

	retval = pthread_mutex_destroy(&decisionQ_mutex);
	if(retval != 0)
	{
		printf("mutex destroy failed, error code - %d\n", retval);
	}

	retval =  pthread_cond_destroy(&condvar);
	if(retval != 0)
	{
		printf("cond destroy failed, error code - %d\n", retval);
	}

	printf("Joined all threads, destroyed mutexes and condition vars...\n");
	printf("Return zero from main()...\n");
	return 0;
}
