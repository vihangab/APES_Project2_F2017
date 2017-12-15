/*****************************************************
 * File: msg_queue_unit_test.c
 * Authors: Virag Gada and Vihanga Bare
 * Description: Unit Tests to test message queues
 ****************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "drivers/pinout.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include "timers.h"
#include "semphr.h"
#include <tmp102.h>
#include <lsm6ds3.h>

//Queue
QueueHandle_t xtestQ;

// Check if queue was successfully initialised
int queue_initialise()
{
      //create your queue
      xtestQ = xQueueCreate(QLEN,sizeof(LogMsg));

      if(xtestQ == NULL)
        return 0;
      else
          return 1;
}


// Check if queue was full
int queue_full()
{
    LogMsg testmsg;
    static int i = 0;
    while(i < 60)
    {
       //write dummy data to queue
       testmsg.sourceId = i++;
       testmsg.requestID = i++;
       testmsg.data = (25+i);
       testmsg.level = i++;
       sprintf(testmsg.payload,"Dummy value is - %f",testmsg.data);
       testmsg.timestamp = time(NULL);

       if(xQueueSendToBack(xlogQ,&testmsg,10) != pdPASS )
       {
            return 1;
       }
       else
           return 0;
       memset(&testmsg,(int)'\0',sizeof(LogMsg));
    }
}


// Task to check if queue was empty
void queue_empty()
{
    LogMsg testmsg;
    while(uxQueueSpacesAvailable(xtestQ) != QLEN )
    {
        if(xQueueReceive(xtestQ,testmsg,portMAX_DELAY) != pdPASS)
        {
            UARTprintf("\r\nQueue is empty\r\n");
        }
        else
        {
            UARTprintf("\r\nQueue has data\r\n");
        }
    }
}


// Main function
int main(void)
{

  // Initialize system clock to 120 MHz
  g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                                SYSCTL_OSC_MAIN |
                                                SYSCTL_USE_PLL |
                                                SYSCTL_CFG_VCO_480), 120000000);
  ASSERT(g_ui32SysClock == SYSTEM_CLOCK);

  // Set up the UART which is connected to the virtual COM port
  UARTStdioConfig(0, 115200, SYSTEM_CLOCK);


  //test if queue was init properly
  if(queue_initialise() == 1)
      UARTprintf("\r\nLogQ creation successful\r\n");
  else
      UARTprintf("\r\nLogQ creation failed\r\n");


  //test if queue is full
  if(queue_full() == 1)
      UARTprintf("\r\nQueue is full\r\n");
  else
      UARTprintf("\r\nQueue is available\r\n");

  //check if queue is empty
  queue_empty();


  return 0;
}
