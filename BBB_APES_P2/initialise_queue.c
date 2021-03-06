#include "main.h"

void initialize_queue(char * qName, mqd_t *msgHandle)
{
	// unlink the queue if it exists - debug
	mq_unlink (qName);

  struct mq_attr attr;
	attr.mq_maxmsg = 20;
	attr.mq_msgsize = sizeof(LogMsg);
	attr.mq_flags = 0;

  printf("Queue created %s\n",qName);
	*msgHandle =  mq_open(qName,O_RDWR | O_CREAT,0666,&attr);

	if (*msgHandle == -1)
	{
		printf("Error Opening %s\n",qName);
		perror("Error - ");
	}
}
