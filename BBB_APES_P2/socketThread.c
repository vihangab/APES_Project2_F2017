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

#define SERVER_IP   ("127.0.0.1")
#define SERVER_PORT (6001)
#define BUFFER_SIZE (50)


void *SocketThread(void * input){

  /* Variable for our socket fd*/
  uint32_t socket_fd;
  uint32_t bytes_sent;
  uint32_t bytes_received;
  struct sockaddr_in remoteAddr;
  char command[BUFFER_SIZE] = {(int)'\0'};

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
    while(1){
      bytes_received = recv(socket_fd,command,BUFFER_SIZE,0);
      if(bytes_received <= 0){
        break;
      }
      printf("Data received on socket- %s\n", command);
      //recv(socket_fd,logmsg2,sizeof(LogMsg),0);
      pthread_mutex_lock(&logQ_mutex);

      if ((bytes_sent = mq_send (logger_queue_handle,(const char*)command, strlen(command), 1)) != 0) //can be changed later to light queue handle
			{
				perror ("[SocketThread] Sending:");
			}
      memset(command,(int)'\0',BUFFER_SIZE);
      /*

      if ((bytes_sent = mq_send (logger_queue_handle,(const char*)&logmsg2, sizeof(LogMsg), 1)) != 0) //can be changed later to light queue handle
			{
				perror ("[LoggerThread] Sending:");
			}*/
      pthread_mutex_unlock(&logQ_mutex);

      memset(logmsg2,(int)'\0',sizeof(LogMsg));
    }
  }
  /*Close*/
  close(socket_fd);
  pthread_ex