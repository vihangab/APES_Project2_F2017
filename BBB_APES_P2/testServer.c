#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdio.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdbool.h>

#define BUFLEN 		256
#define DEVICE_NAME	"/dev/SimpleCharDrv"
#define DEF_PORT	"5005"
#define QLEN 		32

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
  LOGGER_TASK,
  DECISION_TASK
}Sources;

typedef struct logger
{
  Sources sourceId;
  reqCmds requestID;
  double data;
  LogLevel level;
	time_t timestamp;
	char payload[100];
}LogMsg;

int userspace_sock(const char *portnum, int qlen)
{
	int socket_fd = 0;
	int set_reuse_addr = 1; /* ON */
	struct sockaddr_in server;

	/* Create socket TCP */
	if (0 > (socket_fd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		fprintf(stderr, "server failed to create the listening socket\n");
		exit(1);
	}

	if (0 != setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &set_reuse_addr, sizeof(set_reuse_addr)))
	{
		fprintf(stderr, "server failed to set socket options\n");
	}

	bzero(&server, sizeof(server)); 	/*initialise to zero */
	server.sin_family = AF_INET; 		/* socket familey is ipv4 */
	server.sin_addr.s_addr = htonl(INADDR_ANY); 	/*any valid address for binding */
	server.sin_port = htons((unsigned short)atoi(portnum));

	if(server.sin_port == 0)
	{
		fprintf(stderr, "Error getting port number [%s]\n",portnum);
	}

	/*	Bind the socket	*/
	if (0 > bind(socket_fd, (struct sockaddr *)&server, sizeof(server)))
	{
		fprintf(stderr, "server failed to bind\n");
		exit(1);
	}
	/*	Listen on the socket for up qlen */
	if (0 > listen(socket_fd, qlen))
	{
		fprintf(stderr, "server failed to listen\n");
		exit(1);
	}
	else
	{
		fprintf(stdout, "server listening for a connection on port %s\n", portnum);
	}

	return socket_fd;
}


int main(int argc, char **argv)
{
	char	*portnum = DEF_PORT;	/* Default port number	*/
	int	socket_fd = 0;				/* master server socket	*/
	int	client_socket_fd = 0; 		/* socket descriptor to get command from */
  LogMsg logmsg0;
  socket_fd = userspace_sock("6001", QLEN);

	struct sockaddr_in client;
	socklen_t client_addr_len;

	/* Get the size client's address structure */
	client_addr_len = sizeof(client);

  /* Accept a new client */
	if (0 > (client_socket_fd = accept(socket_fd, (struct sockaddr *)&client, &client_addr_len)))
	{
		fprintf(stderr, "Accept failed\n");
	}
	else
	{
		fprintf(stdout, "Accepted a client!\n");

		while(1)
		{		/* write data to socket */
      int ret;
      logmsg0.sourceId = TEMP_TASK;
      logmsg0.requestID = DECIDE;
      logmsg0.timestamp = time(NULL);
      logmsg0.level = INFO;
      logmsg0.data = 29.65;
      sprintf(logmsg0.payload,"Temp value is - %f",logmsg0.data);
      ret = write(client_socket_fd,&logmsg0,sizeof(LogMsg));
      printf("Bytes sent - %d\n",ret);
			if(ret < 0)
			{
				perror("Failed to read the message from the device.");
				return errno;
			}
      sleep(1);
      logmsg0.sourceId = TEMP_TASK;
      logmsg0.requestID = LOG_DATA;
      logmsg0.timestamp = time(NULL);
      logmsg0.level = WARNING;
      logmsg0.data = 29.65;
      sprintf(logmsg0.payload,"Temp value is - %f",logmsg0.data);
      ret = write(client_socket_fd,&logmsg0,sizeof(LogMsg));
      printf("Bytes sent - %d\n",ret);
			if(ret < 0)
			{
				perror("Failed to read the message from the device.");
				return errno;
			}
      sleep(3);
    }
  }
}
