/*****************************************************
 * File: main.c
 * Authors: Vihanga Bare and Virag Gada
 * Description: Source file for TIVA sensors, tasks, queues
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

//#define DEBUG


//Queue Handle
QueueHandle_t xlogQ;

//Timer Handle
TimerHandle_t xtimer;

//semaphore Handle
xSemaphoreHandle temp_task = 0;
xSemaphoreHandle pedo_task = 0;
xSemaphoreHandle xlogQ_mutex = 0;

//Heartbeat semaphores
xSemaphoreHandle temp_hb = 0;
xSemaphoreHandle pedo_hb = 0;
xSemaphoreHandle socket_hb = 0;


//task handles
xTaskHandle xtempTaskHandle;
xTaskHandle xpedoTaskHandle;
xTaskHandle xloggerTaskHandle;
xTaskHandle xsocketTaskHandle;
xTaskHandle xheartbeatTaskHandle;

// TIVA Task declarations
void temperatureTask(void *pvParameters);
void pedometerTask(void *pvParameters);
void loggerTask(void *pvParameters);
void socketTask(void *pvParameters);
void heartbeatTask(void *pvParameters);
void vTimerCallBack(void *);

void vTimerCallBack(void* a)
{
    xSemaphoreGive(temp_task);
    xSemaphoreGive(pedo_task);
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

  //Create timer
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



  //Create binary semaphore for signaling
  vSemaphoreCreateBinary(temp_task);
  vSemaphoreCreateBinary(pedo_task);
  vSemaphoreCreateBinary(pedo_hb);
  vSemaphoreCreateBinary(temp_hb);

  //create a mutex for logQ
  xlogQ_mutex = xSemaphoreCreateMutex();

  // Create tasks
  xTaskCreate(temperatureTask, (const portCHAR *)"Temperature",1024, NULL, 1, xtempTaskHandle);

  xTaskCreate(pedometerTask, (const portCHAR *)"Pedometer",1024, NULL, 1, xpedoTaskHandle);

#ifndef DEBUG
  //Logger Task, this task is only a debug task to test logs from temp and pedo tasks
  xTaskCreate(loggerTask, (const portCHAR *)"Logger",1024, NULL, 1, NULL);
#endif

  xTaskCreate(socketTask, (const portCHAR *)"Socket",1024, NULL, 1, xsocketTaskHandle);

  xTaskCreate(heartbeatTask, (const portCHAR *)"HeartBeat",1024, NULL, 1, xheartbeatTaskHandle);

  UARTprintf("\r\ntemp pedo logger tasks created\r\n");

  UARTprintf("\r\nexit main\r\n");

  // start task scheduler
  vTaskStartScheduler();
  return 0;
}


void socketTask(void *pvParameters)
{
  socketInit();
  while(1);
}


/* Function Name - temperatureTask(void *pvParameters)
 * Function Description - Task to receive data from the temperature sensor
 * */
void temperatureTask(void *pvParameters)
{
    //setup the I2C
    setupTMP102();
    double temp;
    uint32_t timeVal;
    LogMsg temp_msg;

    //task runs forever and needs to be explicity deleted by heartbeat task
    for(;;)
    {
       //wait for a signal
       if(xSemaphoreTake(temp_task,portMAX_DELAY) == pdTRUE)
       {
           //send signal to heartbeat task
           xSemaphoreGive(temp_hb);

           if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
           {

              //read I2C sensor
              readTMP102(&temp);

              //clear contents of log struct, set them to null
              memset(&temp_msg,(int)'\0',sizeof(LogMsg));

              //update log struct with sensor data and timestamp
              temp_msg.sourceId = TEMP_TASK;
              temp_msg.requestID = LOG_DATA;
              temp_msg.data = temp;
              temp_msg.level = INFO;
              sprintf(temp_msg.payload,"Temperature value is - %f",temp);
              timeVal = time(NULL);
              strcpy(temp_msg.timestamp,ctime(&timeVal));

              //Debug prints to check log struct data, commented out
#ifndef
              UARTprintf("\r\n[Temp] source ID: %d\r\n", temp_msg.sourceId);
              UARTprintf("\r\n[Temp] Log Level: %d\r\n", temp_msg.level);
              UARTprintf("\r\n[Temp] Payload: %s  \r\n", temp_msg.payload);
              UARTprintf("\r\n[Temp] Timestamp: %s   %d\r\n", temp_msg.timestamp,sizeof(LogMsg));
#endif

              //add log struct data values to the end of the queue

              if(xQueueSendToBack(xlogQ,&temp_msg,10) != pdPASS )
              {
                  UARTprintf("\r\nQueue is full waiting for 10 ticks \r\n");
              }

           }
           xSemaphoreGive(xlogQ_mutex);
       }

    }

}


/* Function Name - pedometerTask(void *pvParameters)
 * Function Description - Task to receive data from the pedometer sensor
 * */
void pedometerTask(void *pvParameters)
{
    uint16_t steps;
    time_t timeVal;
    LogMsg pedo_msg;

    //setup the I2C
    setupLSM6DS3();

    //task runs forever and needs to be explicity deleted by heartbeat task
    for(;;)
    {
        //wait for a signal
        xSemaphoreGive(pedo_hb);
         if(xSemaphoreTake(pedo_task,portMAX_DELAY) == pdTRUE)
          {

              if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
              {
                  //read the I2C sensor data
                   readStepCount(&steps);
                   pedo_msg.sourceId = PEDO_TASK;
                   pedo_msg.requestID = LOG_DATA;
                   pedo_msg.data = steps;
                   pedo_msg.level = INFO;
                   sprintf(pedo_msg.payload,"Step count - %d",steps);
                   timeVal = time(NULL);
                   strcpy(pedo_msg.timestamp,ctime(&timeVal));
#ifndef
              UARTprintf("\r\n[Pedometer] source ID: %d\r\n", pedo_msg.sourceId);
              UARTprintf("\r\n[Pedometer] Log Level: %d\r\n", pedo_msg.level);
              UARTprintf("\r\n[Pedometer] Payload: %s  \r\n", pedo_msg.payload);
              UARTprintf("\r\n[Pedometer] Timestamp: %s   %d\r\n", pedo_msg.timestamp,sizeof(LogMsg));
#endif


                   // add log struct data vales to the end of the queue
                   if(xQueueSendToBack(xlogQ,&pedo_msg,10) != pdPASS )
                   {
                       UARTprintf("\r\nQueue is full task waiting for 10 ticks\r\n");
                   }

              }
              xSemaphoreGive(xlogQ_mutex);
          }
    }

}

/* Function Name - loggerTask(void *pvParameters)
 * Function Description - Task created for debugging if temp and pedometer tasks are able to write data to queues
 *                        Also, to check if data can be read from the queues, if queues are full or empty
 * */
void loggerTask(void *pvParameters)
{
#ifndef DEBUG
    time_t timeVal;
    LogMsg log_msg;
    for(;;)
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
                        UARTprintf("\r\n[Logger] source ID: %d\r\n", log_msg.sourceId);
                        UARTprintf("\r\n[Logger] Log Level: %d\r\n", log_msg.level);
                        UARTprintf("\r\n[Logger] Payload: %s  \r\n", log_msg.payload);
                        UARTprintf("\r\n[Logger] Timestamp: %s   %d\r\n", log_msg.timestamp,sizeof(LogMsg));
                    }
            }
            xSemaphoreGive(xlogQ_mutex);
       }
    }
#endif
}


/* Function Name - heartbeatTask(void *pvParameters)
 * Function Description - Task created for receiving heartbeats from temp and pedometer tasks.
 *                        If heartbeat not received this task will delete rest of the tasks
 * */
void heartbeatTask(void *pvParameters)
{
    //this is the flag check which decides to kill other tasks
    exitflag = 0;
    LogMsg hb_msg;
    time_t timeVal;
    for(;;)
    {
       //wait for a signal from Temp, Pedometer tasks
       if(xSemaphoreTake(temp_hb,5000) == pdTRUE)
       {
           UARTprintf("\r\nHeartbeat Received from temp task\r\n");
       }
       else
       {
           if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
           {
               UARTprintf("\r\nHeartbeat not Received from Temp task\r\n");

               //creating a log payload to send over the socket to Beaglebone to shut down beaglebone system as well
               hb_msg.sourceId = MAIN_TASK;
               hb_msg.requestID = SYSTEM_SHUTDOWN;
               hb_msg.data = 99.9999;
               hb_msg.level = ALERT;
               sprintf(hb_msg.payload,"Heartbeat not received from Temp Task");
               timeVal = time(NULL);
               strcpy(hb_msg.timestamp,ctime(&timeVal));
               if(xQueueSendToBack(xlogQ,&hb_msg,10) != pdPASS )
               {
                   UARTprintf("\r\nQueue is full task blocks for port max delay\r\n");
               }

           }
           xSemaphoreGive(xlogQ_mutex);
           vTaskDelay(100);
           exitflag = 0x01;
       }
       if(xSemaphoreTake(pedo_hb,5000) == pdTRUE)
       {
          if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
          {

              UARTprintf("\r\nHeartbeat not Received from Pedometer task\r\n");

              //creating a log payload to send over the socket to Beaglebone to shut down beaglebone system as well
              vTaskDelay(100);
              hb_msg.sourceId = MAIN_TASK;
              hb_msg.requestID = SYSTEM_SHUTDOWN;
              hb_msg.data = 99.9999;
              hb_msg.level = ALERT;
              sprintf(hb_msg.payload,"Heartbeat not received from Pedo Task");
              timeVal = time(NULL);
              strcpy(hb_msg.timestamp,ctime(&timeVal));
              if(xQueueSendToBack(xlogQ,&hb_msg,10) != pdPASS )
              {
                  UARTprintf("\r\nQueue is full task blocks for port max delay\r\n");
              }
              exitflag = 0x01;
          }
          xSemaphoreGive(xlogQ_mutex);
       }
       else
       {
          UARTprintf("\r\nHeartbeat not Received from pedo task\r\n");
          exitflag = 0x02;
       }
      if(exitflag != 0)
      {
          //exit flag is zero, not set by any heartbeat so go ahead an delete all tasks
          vTaskDelete(xtempTaskHandle);
          vTaskDelay(100);
          if(eTaskGetState(xtempTaskHandle) == eDeleted)
          {
              UARTprintf("\r\nTemp Task Killed\r\n");
          }

          vTaskDelete(xpedoTaskHandle);
          vTaskDelay(100);
          if(eTaskGetState(xpedoTaskHandle) == eDeleted)
          {
                UARTprintf("\r\nPedometer Task Killed\r\n");
          }

          vTaskDelete(xsocketTaskHandle);
          vTaskDelay(100);
          if(eTaskGetState(xsocketTaskHandle) == eDeleted)
          {
              UARTprintf("\r\nSocket Task Killed\r\n");
          }
          break;
      }
    }
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
