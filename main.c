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


#define MAXLEN 500
#define QLEN 10

struct logdata
{
    uint32_t msgID;
    char *currtime;
    char logdata[MAXLEN];
};

typedef enum
{
  MAIN_TASK,
  TEMP_TASK,
  PEDO_TASK,
  LOGGER_TASK
}Source;


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
  system_clock_rate_hz = ROM_SysCtlClockFreqSet(
                            (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                             SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                            SYSTEM_CLOCK);
  ASSERT(system_clock_rate_hz == SYSTEM_CLOCK);

  // Initialize the GPIO pins for the Launchpad
  // Use Ethernet
  PinoutSet(true, false);

  // Set up the UART which is connected to the virtual COM port
   UARTStdioConfig(0, 57600, SYSTEM_CLOCK);
    //create your queue
    xlogQ = xQueueCreate(QLEN,sizeof(struct logdata));

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
    xTaskCreate(temperatureTask, (const portCHAR *)"Temperature",configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(pedometerTask, (const portCHAR *)"Pedometer",configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(loggerTask, (const portCHAR *)"Logger",configMINIMAL_STACK_SIZE, NULL, 1, NULL);

  UARTprintf("\r\ntemp pedo logger tasks created\r\n");

  UARTprintf("exit main\n");
  vTaskStartScheduler();

  return 0;
}


// Task to receive data from the temperature sensor
void temperatureTask(void *pvParameters)
{

    setupTMP102();
    double temp;
    for(;;)
    {


       //wait for a signal
       if(xSemaphoreTake(temp_task,portMAX_DELAY) == pdTRUE)
       {

           if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
           {
              UARTprintf("\r\nTemp Task received signal\r\n");
              readTMP102(&temp);
              struct logdata temp_msg;
              //int temp1 = (int)temp;
              //memset(&temp_msg.logdata,0,MAXLEN);
              temp_msg.msgID = (int)temp;
              //sprintf(temp_msg.logdata,"temp task data = %d",temp1);
              strcpy(temp_msg.logdata,"test msg from temp task");
              UARTprintf("\r\nTemp=  %d\r\n",(int)temp);
              time_t a = time(NULL);
              temp_msg.currtime = ctime(&a);
              if(xQueueSendToBack(xlogQ,&temp_msg,10) != pdPASS )
              {
                  UARTprintf("\r\nQueue is full task blocks for 10 ticks\r\n");
              }
             UARTprintf("\r\nTemp task completed\r\n");
           }
           xSemaphoreGive(xlogQ_mutex);
       }
       //vTaskDelay(2000);
    }

}


// Task to receive foot step count from the sensor
void pedometerTask(void *pvParameters)
{

    for(;;)
    {
        //wait for a signal

         if(xSemaphoreTake(pedo_task,portMAX_DELAY) == pdTRUE)
          {

              if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
              {
                  UARTprintf("\r\nPedo Task received signal\r\n");
                   struct logdata pedo_msg;
                   strcpy(pedo_msg.logdata,"test msg from pedo task");
                   time_t a = time(NULL);
                   pedo_msg.currtime = ctime(&a);
                   pedo_msg.msgID = PEDO_TASK;
                   if(xQueueSendToBack(xlogQ,&pedo_msg,10) != pdPASS )
                   {
                       UARTprintf("\r\nQueue is full task blocks for 10 ticks\r\n");
                   }
                   //UARTprintf("\r\nPedo task completed\r\n");
              }
              xSemaphoreGive(xlogQ_mutex);
          }

       //vTaskDelay(2000);
    }

}

// Task to receive foot step count from the sensor
void loggerTask(void *pvParameters)
{

    for(;;)
    {
       struct logdata log_msg;
       while(uxQueueSpacesAvailable(xlogQ) != QLEN )
       {
            if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
            {
                    UARTprintf("\r\n Logger task received data\r\n");
                    if(xQueueReceive(xlogQ,&log_msg,portMAX_DELAY) != pdPASS)
                    {
                      UARTprintf("\r\nQueue is empty task blocks for 10 ticks\r\n");
                    }
                    else
                    {
                      UARTprintf("\r\nMsg ID = %d\r\n",log_msg.msgID);
                      UARTprintf("\r\nTimestamp = %s\r\n",log_msg.currtime);
                      UARTprintf("\r\nLog Data = %s\r\n",log_msg.logdata);
                    }
            }
            xSemaphoreGive(xlogQ_mutex);

          //vTaskDelay(2000);
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
