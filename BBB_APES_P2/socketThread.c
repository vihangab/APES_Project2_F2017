/*****************************************************
 * File: socketThread.c
 * Authors: Virag Gada and Vihanga Bare
 * Description: Source file for socket handling task
 ****************************************************/

#include "main.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <memory.h>
#include <dirent.h>

#undef SERVER_LOGGER
#define CLIENT_LOGGER


#define SERVER_IP   "128.138.189.132"//("10.0.0.137")
#define SERVER_PORT (5000)
#define MAX_CONNECTIONS   (100)
#define BACKLOG           (20)

#ifdef SERVER_LOGGER

#define BACKLOG           (20)
#define PORT_NUMBER       (5000)

/* Array of child threads and function descriptiors */
pthread_t child_threads[MAX_CONNECTIONS];
uint32_t client_fd[MAX_CONNECTIONS];

/* Variable for our socket fd*/
uint32_t sockfd;

void *respondClient(void * num);

#endif

void *SocketThread(void * input){
  time_t timeVal;
  #ifdef CLIENT_LOGGER
  printf("%s\n","Client side logger");
  /* Variable for our socket fd*/
  uint32_t socket_fd;
  uint32_t bytes_sent;
  uint32_t bytes_received;
  uint32_t i;
  struct sockaddr_in remoteAddr;
  //char command[80] = {(int)'\0'};

  LogMsg *logmsg2;

  /* Create client socket */
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Client failed to create socket\n");
    exit(1);
  }

  memset(&remoteAddr,(int)'\0',sizeof(struct sockaddr_in));
  remoteAddr.sin_family = AF_INET;
  remoteAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
  remoteAddr.sin_port = htons(SERVER_PORT);

  /* Connect to server */
  if(connect(socket_fd,(struct sockaddr*)&remoteAddr,sizeof(struct sockaddr))<0){
    perror("Error Connection:");
    exit(1);
  }else{
    printf("Connected to server\n");
  }
  if(create_log_struct(&logmsg2)!=DONE){
		printf("%s\n","Error creating struct");
	}else{
    /* Receive data */

	/*dummy request */
	memset(logmsg2,(int)'\0',sizeof(LogMsg));
	logmsg2->sourceId= (uint8_t)LOGGER_TASK;
	logmsg2->requestID=(uint8_t) LOG_DATA;
	//logmsg2->data = 0.0000;
	logmsg2->level = (uint8_t)INFO;
	strcpy(logmsg2->payload,"init socket connection from BBG\n");
        //timeVal = time(NULL);
	//strcpy(logmsg2->timestamp,ctime(&timeVal));
        
	/*sending dummy data */
	//printf ("[SocketThread] source ID: %d \n", logmsg2->sourceId);
  	//printf ("[SocketThread] Log Level: %d \n", logmsg2->level);
  	//printf ("[SocketThread] Payload: %s \n", logmsg2->payload);
 	//printf ("[SocketThread] Timestamp: %s \n", logmsg2->timestamp);

  	bytes_sent = write(socket_fd,logmsg2,sizeof(LogMsg));
  	//printf("Bytes sent 1- %d\n",bytes_sent);

	usleep(1000);

	/*bytes_sent = write(socket_fd,logmsg2,sizeof(LogMsg));
        printf("Bytes sent 2- %d\n",bytes_sent);
        usleep(1000);
*/     // memset(logmsg2,(int)'\0',sizeof(LogMsg));
	
	bytes_received = recv(socket_fd,logmsg2,sizeof(LogMsg),0);
	if(bytes_received <= 0){
        perror("[SocketThread] Receiving data over socket");

	}
	else if(!strncmp(logmsg2->payload,"ACK",strlen("ACK")+1))
	{
		printf("received %s\n",logmsg2->payload);
	}
	//usleep(500);
        

	bytes_sent = write(socket_fd,logmsg2,sizeof(LogMsg));
	if(bytes_sent<= 0){
  perror("[SocketThread] Sending data over socket");

  }
  usleep(1000);
  

  while(1){
    bytes_received = recv(socket_fd,logmsg2,sizeof(LogMsg),0);
    if(bytes_received <= 0){
      perror("[SocketThread] Receiving data over socket");
      break;
    }else{


  /*Send to Logger Task */
  //pthread_mutex_lock(&logQ_mutex);
  //printf("received buffer over socket %s\n",command);
    bytes_sent = write(socket_fd,logmsg2,sizeof(LogMsg));
    if(bytes_sent<= 0){
      perror("[SocketThread] Sending data over socket");
    }
    printf("Count - %d\n", bytes_sent + i++);
    usleep(1000);
      /*if ((bytes_sent = mq_send (logger_queue_handle,(const char*)command, strlen(command), 1)) != 0) //can be changed later to light queue handle
			{
				perror ("[SocketThread] Sending:");
			}
      */
      printf ("[SocketThread] source ID: %d \n", logmsg2->sourceId);
      printf ("[SocketThread] Log Level: %d \n", logmsg2->level);
      printf ("[SocketThread] Payload: %s \n", logmsg2->payload);
      printf ("[SocketThread] Timestamp: %s \n", logmsg2->timestamp);

      if(logmsg2->requestID == LOG_DATA){
        if ((bytes_sent = mq_send (logger_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 1)) != 0) //can be changed later to light queue handle
  			{
  				perror ("[SocketThread] Sending Log");
  			}
      }
      //pthread_mutex_unlock(&logQ_mutex);

      /* Send to Decision Task */
      //pthread_mutex_lock(&decisionQ_mutex);
      if(logmsg2->requestID == DECIDE){
        sleep(0.5);
        if ((bytes_sent = mq_send (decision_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 1)) != 0) //can be changed later to light queue handle
  			{
  				perror ("[SocketThread] Sending Decision");
  			}
      }else if(logmsg2->requestID == SYSTEM_SHUTDOWN){
        sleep(0.5);
        if ((bytes_sent = mq_send (decision_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 2)) != 0) //can be changed later to light queue handle
        {
          perror ("[SocketThread] Sending Decision");
        }
        sleep(1);
        if ((bytes_sent = mq_send (logger_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 2)) != 0) //can be changed later to light queue handle
        {
          perror ("[SocketThread] Sending Log");
        }
      }
      if(SIGINT_EVENT){
        printf("%s\n","Exiting socket");
        break;
      }
      //pthread_mutex_unlock(&decisionQ_mutex);

      //memset(command,(int)'\0',MAX_SEND_BUFFER);
      //memset(logmsg2,(int)'\0',sizeof(LogMsg));
	    usleep(1000);
      }
    }
  }
  /*Close*/
  close(socket_fd);
  #endif

  #ifdef SERVER_LOGGER
  printf("%s\n","Server side logger");
      /*Variables*/
    long availableSlot = 0;
    struct sockaddr_in *selfAddr;
    struct sockaddr_in fromAddr;
    socklen_t fromAddrSize;
    char buffer[MAX_SEND_BUFFER];

    pthread_attr_t attr;

    selfAddr = (struct sockaddr_in *)calloc(1,sizeof(struct sockaddr_in));

    (*selfAddr).sin_family = AF_INET;           //address family
    (*selfAddr).sin_port = htons(PORT_NUMBER); //sets port to network byte order
    (*selfAddr).sin_addr.s_addr = htonl(INADDR_ANY); //sets local address

    /*Create Socket*/
    if((sockfd = socket((*selfAddr).sin_family,SOCK_STREAM,0))< 0) {
     printf("Unable to create socket\n");
     exit(1);
    }
    printf("Socket Created\n");

    /*Call Bind*/
    if(bind(sockfd,(struct sockaddr *)selfAddr,sizeof(*selfAddr))<0) {
     printf("Unable to bind\n");
     exit(1);
    }
    printf("Socket Binded\n");

    /* Listen for incomming connections */
    if(listen(sockfd,BACKLOG)!=0){
     printf("%s\n","Listen Error");
     exit(1);
    }

    memset(&fromAddr,(int)'\0',sizeof(fromAddr));
    memset(buffer,(int)'\0',sizeof(buffer));
    fromAddrSize = sizeof(fromAddr);
    memset(client_fd,-1,sizeof(client_fd));
    pthread_attr_init(&attr);

    /*listen*/
    while(1) {
     printf("waiting for connections..\n");
     /*Accept*/
     if((client_fd[availableSlot] = accept(sockfd,(struct sockaddr *)&fromAddr,&fromAddrSize)) < 0)
     {
       printf("Failed to accept connection\n");
       break;
     }else{
       /* Create new thread to handle the client */
       pthread_create(&child_threads[availableSlot],&attr,respondClient,(void *)availableSlot);
     }
     /* Add client fd to a position in the array which is empty */
     while (client_fd[availableSlot]!=-1) {
       availableSlot = (availableSlot+1)%MAX_CONNECTIONS;
     }
     printf("Slot number - %ld\n", availableSlot);
    }
  #endif

  pthread_exit(NULL);
}

#ifdef SERVER_LOGGER
  /* Thread function to respond to client request */
  void *respondClient(void * num){

    int client_no = (int )(long) num;
    LogMsg *logmsg2;
    uint32_t bytes_sent;
    uint32_t bytes_received;
    int i;

    printf("Thread %d created\n",client_no);
    if(create_log_struct(&logmsg2)!=DONE){
      printf("%s\n","Error creating struct");
    }else{
      /* Receive data */
      while(1){
        bytes_received = recv(client_fd[client_no],logmsg2,sizeof(LogMsg),0);
        if(bytes_received <= 0){
          perror("[SocketThread] Receiving data over socket");
          break;
        }else{


      /*Send to Logger Task */
      //pthread_mutex_lock(&logQ_mutex);
      //printf("received buffer over socket %s\n",command);
      /*  bytes_sent = write(socket_fd,logmsg2,sizeof(LogMsg));
        if(bytes_sent<= 0){
          perror("[SocketThread] Sending data over socket");
        }
        printf("Count - %d\n", bytes_sent + i++);
        usleep(1000);*/
          /*if ((bytes_sent = mq_send (logger_queue_handle,(const char*)command, strlen(command), 1)) != 0) //can be changed later to light queue handle
          {
            perror ("[SocketThread] Sending:");
          }
          */
          //printf ("[SocketThread] source ID: %d \n", logmsg2->sourceId);
        //  printf ("[SocketThread] Log Level: %d \n", logmsg2->level);
          printf ("[SocketThread] Payload: %s \n", logmsg2->payload);
          //printf ("[SocketThread] Timestamp: %s \n", logmsg2->timestamp);

          if(logmsg2->requestID == LOG_DATA){
            if ((bytes_sent = mq_send (logger_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 1)) != 0) //can be changed later to light queue handle
            {
              perror ("[SocketThread] Sending Log");
            }
          }
          //pthread_mutex_unlock(&logQ_mutex);

          /* Send to Decision Task */
          //pthread_mutex_lock(&decisionQ_mutex);
          if(logmsg2->requestID == DECIDE){
            sleep(0.5);
            if ((bytes_sent = mq_send (decision_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 1)) != 0) //can be changed later to light queue handle
            {
              perror ("[SocketThread] Sending Decision");
            }
          }else if(logmsg2->requestID == SYSTEM_SHUTDOWN){
            sleep(0.5);
            if ((bytes_sent = mq_send (decision_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 2)) != 0) //can be changed later to light queue handle
            {
              perror ("[SocketThread] Sending Decision");
            }
            sleep(1);
            if ((bytes_sent = mq_send (logger_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 2)) != 0) //can be changed later to light queue handle
            {
              perror ("[SocketThread] Sending Log");
            }
          }
          if(SIGINT_EVENT){
            printf("%s\n","Exiting socket");
            break;
          }
          //pthread_mutex_unlock(&decisionQ_mutex);

          //memset(command,(int)'\0',MAX_SEND_BUFFER);
          //memset(logmsg2,(int)'\0',sizeof(LogMsg));
            usleep(1000);
          }
        }
     }

    /* Wait for data, do something */

    //sleep(10);
    printf("Thread %d exiting\n",client_no);

    /* Close the thread socket */
    close(client_fd[client_no]);

    /* Set its value to -1 to use this position for some other thread */
    client_fd[client_no]=-1;
    pthread_exit(NULL);
  }
#endif
