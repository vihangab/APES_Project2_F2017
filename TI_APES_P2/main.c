/*****************************************************
 * File: main.c
 * Authors: Virag Gada and Vihanga Bare
 * Description: Source file for APES Project 2
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
QueueHandle_t xlogQ;

//Timer
TimerHandle_t xtimer;

//semaphore
xSemaphoreHandle temp_task = 0;
xSemaphoreHandle pedo_task = 0;
xSemaphoreHandle xlogQ_mutex = 0;



// Demo Task declarations
void temperatureTask(void *pvParameters);
void pedometerTask(void *pvParameters);
void loggerTask(void *pvParameters);
void socketTask(void *pvParameters);
void vTimerCallBack(void *);

void vTimerCallBack(void* a)
{
    xSemaphoreGive(temp_task);
    xSemaphoreGive(pedo_task);
}



// Main function
int main(void)
{

  //SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);
  // Initialize system clock to 120 MHz
  g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                                SYSCTL_OSC_MAIN |
                                                SYSCTL_USE_PLL |
                                                SYSCTL_CFG_VCO_480), 120000000);
  ASSERT(g_ui32SysClock == SYSTEM_CLOCK);

  // Initialize the GPIO pins for the Launchpad
  // Use Ethernet
  PinoutSet(true, false);

  // Set up the UART which is connected to the virtual COM port
  UARTStdioConfig(0, 115200, SYSTEM_CLOCK);
  //create your queue


  xlogQ = xQueueCreate(QLEN,sizeof(LogMsg));

  if(xlogQ == NULL)
  {
    UARTprintf("\r\nLogQ creation failed\r\n");
  }
  //create timer
  xtimer = xTimerCreate("Systimer",2000,pdTRUE,(void*)0,vTimerCallBack);
  if(xtimer == NULL)
  {
    UARTprintf("\r\nTimer Creation Failed\r\n");
  }
  else
  {
    if(xTimerStart(xtimer,0)!=pdPASS)
      {
        UARTprintf("\r\nTimer Start Failed\r\n");
      }
  }



  //create binary semaphore for signaling
  vSemaphoreCreateBinary(temp_task);
  vSemaphoreCreateBinary(pedo_task);

  //create a mutex for logQ
  xlogQ_mutex = xSemaphoreCreateMutex();

  // Create tasks
  xTaskCreate(temperatureTask, (const portCHAR *)"Temperature",1024, NULL, 1, NULL);

  xTaskCreate(pedometerTask, (const portCHAR *)"Pedometer",1024, NULL, 1, NULL);

  //xTaskCreate(loggerTask, (const portCHAR *)"Logger",1024, NULL, 1, NULL);

  xTaskCreate(socketTask, (const portCHAR *)"Socket",1024, NULL, 1, NULL);

  UARTprintf("\r\ntemp pedo logger tasks created\r\n");

  UARTprintf("exit main\n");
  vTaskStartScheduler();
  return 0;
}


void socketTask(void *pvParameters)
{
  //socketInit();

  while(1);
}


// Task to receive data from the temperature sensor
void temperatureTask(void *pvParameters)
{
    setupTMP102();
    double temp;
    LogMsg temp_msg;
    for(;;)
    {
       //wait for a signal
       if(xSemaphoreTake(temp_task,portMAX_DELAY) == pdTRUE)
       {

           if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
           {
              //UARTprintf("\r\nTemp Task received signal\r\n");
              readTMP102(&temp);
              temp_msg.sourceId = TEMP_TASK;
              temp_msg.requestID = LOG_DATA;
              temp_msg.data = temp;
              temp_msg.level = INFO;
              sprintf(temp_msg.payload,"Temperature value is - %f",temp);
              temp_msg.timestamp = time(NULL);

              UARTprintf("\r\n[Temp] source ID: %d \n", temp_msg.sourceId);
              UARTprintf("\r\n[Temp] Log Level: %d \n", temp_msg.level);
              UARTprintf("\r\n[Temp] Payload: %s   \n", temp_msg.payload);
              UARTprintf("\r\n[Temp] Timestamp: %s \n", ctime(&temp_msg.timestamp));
              if(xQueueSendToBack(xlogQ,&temp_msg,10) != pdPASS )
              {
                  UARTprintf("\r\nQueue is full task blocks for 10 ticks\r\n");
              }
             //UARTprintf("\r\nTemp task completed\r\n");
           }
           xSemaphoreGive(xlogQ_mutex);
           memset(&temp_msg,(int)'\0',sizeof(LogMsg));
           //vTaskDelay(2000);
       }

    }

}


// Task to receive foot step count from the sensor
void pedometerTask(void *pvParameters)
{
    uint16_t steps;
    LogMsg pedo_msg;
    setupLSM6DS3();
    //UARTprintf("Step count - %d\n",steps);
    for(;;)
    {
        //wait for a signal

         if(xSemaphoreTake(pedo_task,portMAX_DELAY) == pdTRUE)
          {

              if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
              {
                   //UARTprintf("\r\nPedo Task received signal\r\n");
                   readStepCount(&steps);
                   pedo_msg.sourceId = PEDO_TASK;
                   pedo_msg.requestID = LOG_DATA;
                   pedo_msg.data = steps;
                   pedo_msg.level = INFO;
                   sprintf(pedo_msg.payload,"Step count - %d",steps);
                   pedo_msg.timestamp = time(NULL);
                   if(xQueueSendToBack(xlogQ,&pedo_msg,10) != pdPASS )
                   {
                       UARTprintf("\r\nQueue is full task blocks for 10 ticks\r\n");
                   }
                   //UARTprintf("\r\nPedo task completed\r\n");

              }
              xSemaphoreGive(xlogQ_mutex);
              //vTaskDelay(2000);
              memset(&pedo_msg,(int)'\0',sizeof(LogMsg));
          }

       //vTaskDelay(2000);
    }

}

// Task to receive foot step count from the sensor
void loggerTask(void *pvParameters)
{
    //LogMsg log_msg;
    /*for(;;)
    {
       while(uxQueueSpacesAvailable(xlogQ) != QLEN )
       {
            if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
            {
                    //UARTprintf("\r\n Logger task received data\r\n");
                    if(xQueueReceive(xlogQ,&log_msg,portMAX_DELAY) != pdPASS)
                    {
                      UARTprintf("\r\nQueue is empty task blocks for 10 ticks\r\n");
                    }
                    else
                    {
                      UARTprintf("\r\nMsg ID = %d\r\n",log_msg.sourceId);
                      UARTprintf("\r\nTimestamp = %s\r\n",ctime(&log_msg.timestamp));
                      UARTprintf("\r\nLog Data = %s\r\n",log_msg.payload);
                      //logmsg = &log_msg;
                      logmsg->sourceId = log_msg.sourceId;;
                      logmsg->requestID = log_msg.requestID;
                      logmsg->data = log_msg.data;
                      logmsg->level = log_msg.level;
                      sprintf(logmsg->payload,"Step count - %d",log_msg.payload);
                      logmsg->timestamp = time(NULL);



                    }
            }
            xSemaphoreGive(xlogQ_mutex);

          //vTaskDelay(2000);
       }
    }*/
}


/*  ASSERT() Error function
 *  failed ASSERTS() from driverlib/debug.h are executed in this function
 */
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}
