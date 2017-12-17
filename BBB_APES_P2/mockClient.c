/***********************************************************
 * File: mockClients.c
 * Authors: Virag Gada and Vihanga Bare
 * Description: Source file for testing multithreaded server
 ***********************************************************/

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* CONSTANTS =============================================================== */
#define SERVER_ADDR "localhost"
#define SERVER_PORT 5000
#define BUFFER_SIZE 1024
#define MESSAGE "ramblin wreck"
uint32_t dataSet[]={1,35,1000,123,12};
char *payloads[]={"Value is 1","Value is 35","Value is 1000","Value is 123","Value is 12"};

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

int main(int argc, char **argv) {

  int socket_fd = 0;
  struct sockaddr_in server_socket_addr;
  char buffer[BUFFER_SIZE];

  time_t timeVal;
  if(argc != 2){
    printf("%s\n","Usage ./mockclient <number>");
    exit(1);
  }
  uint32_t val = atoi(argv[1])%5;
  // Converts localhost into 0.0.0.0
  struct hostent *he = gethostbyname(SERVER_ADDR);
  unsigned long server_addr_nbo = *(unsigned long *)(he->h_addr_list[0]);

  // Create socket (IPv4, stream-based, protocol likely set to TCP)
  if (0 > (socket_fd = socket(AF_INET, SOCK_STREAM, 0))) {
    fprintf(stderr, "client failed to create socket\n");
    exit(1);
  }

  // Configure server socket address structure (init to zero, IPv4,
  // network byte order for port and address)
  bzero(&server_socket_addr, sizeof(server_socket_addr));
  server_socket_addr.sin_family = AF_INET;
  server_socket_addr.sin_port = htons(SERVER_PORT);
  server_socket_addr.sin_addr.s_addr = server_addr_nbo;

  // Connect socket to server
  if (0 > connect(socket_fd, (struct sockaddr *)&server_socket_addr, sizeof(server_socket_addr))) {
    fprintf(stderr, "client failed to connect to %s:%d!\n", SERVER_ADDR, SERVER_PORT);
    close(socket_fd);
    exit(1);
  } else {
    fprintf(stdout, "client connected to to %s:%d!\n", SERVER_ADDR, SERVER_PORT);
  }

  LogMsg logmsg;

  logmsg.sourceId = val;
  logmsg.requestID = 0;
  logmsg.data = dataSet[val];
  logmsg.level = val;
  timeVal = time(NULL);
  strcpy(logmsg.timestamp,ctime(&timeVal));

  strcpy(logmsg.payload,payloads[val]);
  // Send echo message
  if (0 > send(socket_fd, &logmsg, sizeof(LogMsg), 0)) {
    fprintf(stderr, "client failed to send echo message");
    close(socket_fd);
    exit(1);
  }

  sleep(5);

  // Close the socket and return the response length (in bytes)
  close(socket_fd);
  return 0;
}
