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

#define SERVER_IP   "127.0.0.1"//("10.0.0.137")
#define SERVER_PORT (5000)


void *SocketThread(void * input){

  /* Variable for our socket fd*/
  uint32_t socket_fd;
  uint32_t bytes_sent;
  uint32_t bytes_received;
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
	logmsg2->sourceId= LOGGER_TASK;
	logmsg2->requestID= LOG_DATA;
	logmsg2->data = 0.0000;
	logmsg2->level = INFO;
	strcpy(logmsg2->payload,"init socket connection from BBG\n");
	logmsg2->timestamp = time(NULL);

	/*sending dummy data */
	/*printf ("[SocketThread] source ID: %d \n", logmsg2->sourceId);
  printf ("[SocketThread] Log Level: %d \n", logmsg2->level);
  printf ("[SocketThread] Payload: %s \n\n", logmsg2->payload);
  printf ("[SocketThread] Timestamp: %s \n", ctime(&logmsg2->timestamp));

  int bytes_sent = write(socket_fd,logmsg2,sizeof(LogMsg));
  printf("Bytes sent 1- %d\n",bytes_sent);

	usleep(1000);*/

	/*bytes_sent = write(socket_fd,logmsg2,sizeof(LogMsg));
        printf("Bytes sent 2- %d\n",bytes_sent);
        usleep(1000);

	bytes_received = recv(socket_fd,logmsg2,sizeof(LogMsg),0);
	if(bytes_received <= 0){
        perror("[SocketThread] Receiving data over socket");

	}
	else if(!strncmp(logmsg2->payload,"ACK",strlen("ACK")+1))
	{
		printf("received %s\n",logmsg2->payload);
	}
	usleep(1000);
	bytes_sent = write(socket_fd,logmsg2,sizeof(LogMsg));
		if(bytes_sent<= 0){
        perror("[SocketThread] Sending data over socket");

        }
        usleep(1000);
*/
    while(1){
      bytes_received = recv(socket_fd,logmsg2,sizeof(LogMsg),0);
      if(bytes_received <= 0){
        perror("[SocketThread] Receiving data over socket");
        break;
      }else{


	/*Send to Logger Task */
        //pthread_mutex_lock(&logQ_mutex);
	//printf("received buffer over socket %s\n",command);

        /*if ((bytes_sent = mq_send (logger_queue_handle,(const char*)command, strlen(command), 1)) != 0) //can be changed later to light queue handle
  			{
  				perror ("[SocketThread] Sending:");
  			}
        */
        //printf ("[SocketThread] source ID: %d \n", logmsg2->sourceId);
      //  printf ("[SocketThread] Log Level: %d \n", logmsg2->level);
        printf ("[SocketThread] Payload: %s \n", logmsg2->payload);
        //printf ("[SocketThread] Timestamp: %s \n", ctime(&logmsg2->timestamp));

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
  pthread_exit(NULL);
}
