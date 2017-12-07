/*****************************************************
 * File: main.c
 * Authors: Virag Gada and Vihanga Bare
 * Description: Source file for APES Project 2
 ****************************************************/

#include <tmp102.h>
#include "main.h"

// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


// Demo Task declarations
void temperatureTask(void *pvParameters);
void pedometerTask(void *pvParameters);

// Main function
int main(void)
{

    return 0;
}


// Task to receive data from the temperature sensor
void temperatureTask(void *pvParameters)
{

}


// Task to receive foot step count from the sensor
void pedometerTask(void *pvParameters)
{

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
