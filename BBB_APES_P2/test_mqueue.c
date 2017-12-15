/*
	Filename : test_mqueue.c
	File Description : Source files unit tests for POSIX Msg Queues
	File Author: Vihanga Bare and Virag Gada
	Tools used : Ubuntu 16.04.2, GCC version 5.4.0, cmocka, cmake	
*/

#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<mqueue.h>
#include<sys/stat.h>
#include<errno.h>
#include<sys/types.h>

#include "msg_queue.h"
#include "i2c_lib.h"

/*Function - Unit test to check if queue was initialised successfully*/
void test_initialise_queue_fail(void **state)
{
	//initialise queue attributes
	i2c_state status;
	attr.mq_maxmsg = 20;
	attr.mq_msgsize = MAX_SEND_BUFFER;
	attr.mq_flags = 0;
	
	data_queue_handle =  mq_open(QLog,O_RDWR | O_CREAT,0666,&attr);
	main_queue_handle = mq_open(QMain,O_RDWR | O_CREAT,0666,&attr);
	
	if(data_queue_handle == -1 || main_queue_handle == -1)
	{
		status = ERROR;
	}

    assert_int_equal(status,ERROR);
}

void test_initialise_queue_success(void **state)
{
	//initialise queue attributes
	i2c_state status;
	attr.mq_maxmsg = 20;
	attr.mq_msgsize = MAX_SEND_BUFFER;
	attr.mq_flags = 0;
	
	data_queue_handle =  mq_open(QLog,O_RDWR | O_CREAT,0666,&attr);
	main_queue_handle = mq_open(QMain,O_RDWR | O_CREAT,0666,&attr);
	
	if(data_queue_handle == -1 || main_queue_handle == -1)
	{
		status = ERROR;
	}

    assert_int_equal(status,SUCCESS);
}

void test_queue_send_receive_success(void **state)
{
	//initialise queue attributes
	i2c_state status;
	
	char buffer[MAX_SEND_BUFFER] = "this is test string";
	uint32_t bytes_read = 0;
	uint32_t i,j, count;
	count = 20;
	
	initialise_queue();
	
	for(i = 0; i < count;i++) 
	{
		if ((bytes_read = mq_send(data_queue_handle,buffer, strlen(buffer)+1, i)) != 0)
		{
			status = ERROR;
		}
		else
			status = SUCCESS;
	}
	
	mq_getattr(data_queue_handle, &attr);
	while(attr.mq_curmsgs > 0)
	{
		if ((bytes_read = mq_receive(data_queue_handle,buffer, MAX_SEND_BUFFER, &j)) != 0)
		{
			status = ERROR;
		}
		else
			status = SUCCESS;
		mq_getattr(data_queue_handle, &attr);
	}
    assert_int_equal(status,SUCCESS);
}

void test_queue_send_receive_fail(void **state)
{
	//initialise queue attributes
	i2c_state status;
	
	char buffer[MAX_SEND_BUFFER] = "this is test string";
	uint32_t bytes_read = 0;
	uint32_t i,j, count;
	count = 20;
	
	initialise_queue();
	
	for(i = 0; i < count;i++) 
	{
		if ((bytes_read = mq_send(data_queue_handle,buffer,strlen(buffer)+1, i)) != 0)
		{
			status = ERROR;
		}
		else
			status = SUCCESS;
	}
	
	mq_getattr(data_queue_handle, &attr);
	while(attr.mq_curmsgs > 0)
	{
		if ((bytes_read = mq_receive(data_queue_handle,buffer, MAX_SEND_BUFFER, &j)) != 0)
		{
			status = ERROR;
		}
		else
			status = SUCCESS;
		mq_getattr(data_queue_handle, &attr);
	}
    assert_int_equal(status,ERROR);
}


int main(int argc, char **argv)
{
  const struct CMUnitTest tests[] = 
  {
	//unit test framework struct	
	cmocka_unit_test(test_initialise_queue_fail),
	cmocka_unit_test(test_initialise_queue_success),
	cmocka_unit_test(test_queue_send_receive_success),
	cmocka_unit_test(test_queue_send_receive_fail),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
