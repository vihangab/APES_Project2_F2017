//*****************************************************************************
//
// enet_lwip.c - Sample WebServer Application using lwIP.
//
// Copyright (c) 2013-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************
#include "main.h"
#include <enet_lwip.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"
#if LWIP_TCP

static struct tcp_pcb *echo_pcb;
extern LogMsg *logmsg;
extern xSemaphoreHandle xTx_sem;


//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Ethernet with lwIP (enet_lwip)</h1>
//!
//! This example application demonstrates the operation of the Tiva
//! Ethernet controller using the lwIP TCP/IP Stack.  DHCP is used to obtain
//! an Ethernet address.  If DHCP times out without obtaining an address,
//! AutoIP will be used to obtain a link-local address.  The address that is
//! selected will be shown on the UART.
//!
//! UART0, connected to the ICDI virtual COM port and running at 115,200,
//! 8-N-1, is used to display messages from this application. Use the
//! following command to re-build the any file system files that change.
//!
//!     ../../../../tools/bin/makefsfile -i fs -o enet_fsdata.h -r -h -q
//!
//! For additional details on lwIP, refer to the lwIP web page at:
//! http://savannah.nongnu.org/projects/lwip/
//
//*****************************************************************************

//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)

//*****************************************************************************
//
// Interrupt priority definitions.  The top 3 bits of these values are
// significant with lower values indicating higher priority interrupts.
//
//*****************************************************************************
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0

//*****************************************************************************
//
// The current IP address.
//
//*****************************************************************************
uint32_t g_ui32IPAddress;

//*****************************************************************************
//
// The system clock frequency.
//
//*****************************************************************************
//uint32_t g_ui32SysClock;

//*****************************************************************************
//
// Volatile global flag to manage LED blinking, since it is used in interrupt
// and main application.  The LED blinks at the rate of SYSTICKHZ.
//
//*****************************************************************************
//volatile bool g_bLED;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
/*#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif*/

//*****************************************************************************
//
// Display an lwIP type IP Address.
//
//*****************************************************************************
void
DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    //
    // Convert the IP Address into a string.
    //
    usprintf(pcBuf, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    //
    // Display the string.
    //
    UARTprintf(pcBuf);
}

//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
void
lwIPHostTimerHandler(void)
{
    uint32_t ui32NewIPAddress;

    //
    // Get the current IP address.
    //
    ui32NewIPAddress = lwIPLocalIPAddrGet();

    //
    // See if the IP address has changed.
    //
    if(ui32NewIPAddress != g_ui32IPAddress)
    {
        //
        // See if there is an IP address assigned.
        //
        if(ui32NewIPAddress == 0xffffffff)
        {
            //
            // Indicate that there is no link.
            //
            UARTprintf("Waiting for link.\n");
        }
        else if(ui32NewIPAddress == 0)
        {
            //
            // There is no IP address, so indicate that the DHCP process is
            // running.
            //
            UARTprintf("Waiting for IP address.\n");
        }
        else
        {
            //
            // Display the new IP address.
            //
            UARTprintf("IP Address: ");
            DisplayIPAddress(ui32NewIPAddress);
            UARTprintf("\nOpen a browser and enter the IP address.\n");
        }

        //
        // Save the new IP address.
        //
        g_ui32IPAddress = ui32NewIPAddress;
    }

    //
    // If there is not an IP address.
    //
    if((ui32NewIPAddress == 0) || (ui32NewIPAddress == 0xffffffff))
    {
        //
        // Do nothing and keep waiting.
        //
    }
}

//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Call the lwIP timer handler.
    //
    lwIPTimer(SYSTICKMS);

    //
    // Tell the application to change the state of the LED (in other words
    // blink).
    //
    //g_bLED = true;
}

//*****************************************************************************
//
// This example demonstrates the use of the Ethernet Controller.
//
//*****************************************************************************
void
echo_init(void)
{
  echo_pcb = tcp_new();
  if (echo_pcb != NULL)
  {
    err_t err;

    err = tcp_bind(echo_pcb, IP_ADDR_ANY, 5000);// my port number
    if (err == ERR_OK)
    {
      echo_pcb = tcp_listen(echo_pcb);
      tcp_accept(echo_pcb, echo_accept);
    }
    else
    {
      UARTprintf("\r\n error in bind");
    }
  }
  else
  {
      UARTprintf("\r\n error in tcp new ");
  }
}


err_t
echo_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  err_t ret_err;
  struct echo_state *es;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  /* commonly observed practive to call tcp_setprio(), why? */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  es = (struct echo_state *)mem_malloc(sizeof(struct echo_state));
  if (es != NULL)
  {
    es->state = ES_ACCEPTED;
    es->pcb = newpcb;
    es->retries = 0;
    es->p = NULL;
    /* pass newly allocated es to our callbacks */
    tcp_arg(newpcb, es);
    tcp_recv(newpcb, echo_recv);
    tcp_err(newpcb, echo_error);
    tcp_poll(newpcb, echo_poll, 4);
    ret_err = ERR_OK;
  }
  else
  {
    ret_err = ERR_MEM;
  }
  return ret_err;
}

err_t
echo_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct echo_state *es;
  err_t ret_err;


  es = (struct echo_state *)arg;
  if (p == NULL)
  {
    /* remote host closed connection */
    es->state = ES_CLOSING;
    if(es->p == NULL)
    {
       /* we're done sending, close it */
       echo_close(tpcb, es);
    }
    else
    {
      /* we're not done yet */
      echo_send(tpcb, es);
    }
    ret_err = ERR_OK;
  }
  else if(err != ERR_OK)
  {
    /* cleanup, for unkown reason */
    if (p != NULL)
    {
      es->p = NULL;
      pbuf_free(p);
    }
    ret_err = err;
  }
  else if(es->state == ES_ACCEPTED)
  {
    /* first data chunk in p->payload */
    es->state = ES_RECEIVED;
    /* store reference to incoming pbuf (chain) */

    UARTprintf("\r\n Accept Successful, %s payload size = %d\r\n",p->payload,p->tot_len);
    es->p = p;
    //echo_send(tpcb, es);
    /*dummy request */
    //logmsg->sourceId= LOGGER_TASK;
    //logmsg->requestID= LOG_DATA;
    //logmsg->data = 25.0000;
    //logmsg->level = INFO;
    //strcpy(logmsg->payload,"ACK");
    //logmsg->timestamp = time(NULL);
    p->payload = logmsg;
    UARTprintf("\r\n[TIVA] source ID: %d \n", logmsg->sourceId);
    UARTprintf("\r\n[TIVA] Log Level: %d \n", logmsg->level);
    UARTprintf("\r\n[TIVA] Payload: %s   \n", logmsg->payload);
    UARTprintf("\r\n[TIVA] Timestamp: %s \n", ctime(&logmsg->timestamp));

    //echo_send(tpcb, es);
    tcp_write(tpcb, logmsg,sizeof(LogMsg),1);
    tcp_output(tpcb);
    /* install send completion notifier */
    ret_err = ERR_OK;
  }
  else if(es->state == ES_CLOSING)
  {
    /* odd case, remote side closing twice, trash data */
    tcp_recved(tpcb, p->tot_len);
    UARTprintf("\r\n inside closing = %d ",p->tot_len);
    es->p = NULL;
    pbuf_free(p);
    ret_err = ERR_OK;
  }
  else if (es->state == ES_RECEIVED)
  {
        /* read some more data */
        if(es->p == NULL)
        {
          es->p = p;
        }
        else
        {
            /*dummy request */
               //logmsg->sourceId= LOGGER_TASK;
               //logmsg->requestID= LOG_DATA;
               //logmsg->data = 25.0000;
               //logmsg->level = INFO;
               //strcpy(logmsg->payload,"ACK");
               //logmsg->timestamp = time(NULL);

            //if(xSemaphoreTake(xTx_sem,20) == 1){

               p->payload = logmsg;
               UARTprintf("\r\n[TIVA] source ID: %d \n", logmsg->sourceId);
               UARTprintf("\r\n[TIVA] Log Level: %d \n", logmsg->level);
               UARTprintf("\r\n[TIVA] Payload: %s   \n", logmsg->payload);
               UARTprintf("\r\n[TIVA] Timestamp: %s \n", ctime(&logmsg->timestamp));

               //echo_send(tpcb, es);
               tcp_write(tpcb, logmsg,sizeof(LogMsg),1);
               tcp_output(tpcb);
          //tcp_write(tpcb, logmsg,sizeof(LogMsg),1);
        }
        ret_err = ERR_OK;
  }
  else
    {
      /* unkown es->state, trash data  */
      tcp_recved(tpcb, p->tot_len);
      UARTprintf("\r\n inside else state %d and state is %d",p->tot_len,es->state);
      es->p = NULL;
      pbuf_free(p);
      ret_err = ERR_OK;
    }

  return ret_err;
}

void
echo_error(void *arg, err_t err)
{
  struct echo_state *es;

  LWIP_UNUSED_ARG(err);

  es = (struct echo_state *)arg;
  if (es != NULL)
  {
    mem_free(es);
  }
}

err_t
echo_poll(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  struct echo_state *es;

  es = (struct echo_state *)arg;
  if (es != NULL)
  {
    if (es->p != NULL)
    {
      /* there is a remaining pbuf (chain)  */
      UARTprintf("\r\n data sending in poll %s ",(es->p)->payload);

      tcp_write(tpcb, logmsg,sizeof(LogMsg),1);
      //tcp_output(tpcb);
      //echo_send(tpcb, es);

     // tcp_sent(tpcb, echo_sent);
      //echo_send(tpcb, es);
    }
    ret_err = ERR_OK;
  }
  else
  {
    /* nothing to be done */
    //tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

err_t
echo_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  struct echo_state *es;

  LWIP_UNUSED_ARG(len);

  es = (struct echo_state *)arg;
  es->retries = 0;

  if(es->p != NULL)
  {
    /* still got pbufs to send */
    //strcpy((es->p)->payload,"Here I am sending something");
    UARTprintf("\r\n printing what I am sending %s ",(es->p)->payload);

    //tcp_sent(tpcb, echo_sent);
    echo_send(tpcb, es);
    //es->state = ES_CLOSING;
  }
  else
  {
    /* no more pbufs to send */
    if(es->state == ES_CLOSING)
    {
      echo_close(tpcb, es);
    }
  }
  return ERR_OK;
}

void
echo_send(struct tcp_pcb *tpcb, struct echo_state *es)
{
  struct pbuf *ptr;
  err_t wr_err = ERR_OK;

  while ((wr_err == ERR_OK) &&
         (es->p != NULL) &&
         (es->p->len <= tcp_sndbuf(tpcb)))
  {

  ptr = es->p;

  /* enqueue data for transmission */

  wr_err = tcp_write(tpcb, ptr->payload,sizeof(LogMsg),1);
  if (wr_err == ERR_OK)
  {
     u16_t plen;
      u8_t freed;

     plen = ptr->len;
     /* continue with next pbuf in chain (if any) */
     es->p = ptr->next;
     if(es->p != NULL)
     {
       /* new reference! */
       pbuf_ref(es->p);
     }
     /* chop first pbuf from chain */
      do
      {
        /* try hard to free pbuf */
        freed = pbuf_free(ptr);
      }
      while(freed == 0);
     /* we can read more data now */
     tcp_recved(tpcb, plen);
   }
   else if(wr_err == ERR_MEM)
   {
      /* we are low on memory, try later / harder, defer to poll */
     es->p = ptr;
   }
   else
   {
     /* other problem ?? */
   }
  }
}

void
echo_close(struct tcp_pcb *tpcb, struct echo_state *es)
{
  tcp_arg(tpcb, NULL);
  //tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  //tcp_poll(tpcb, NULL, 0);

  if (es != NULL)
  {
    mem_free(es);
  }
  tcp_close(tpcb);
}

#endif /* LWIP_TCP */

int
socketInit(void)
{
    uint32_t ui32User0, ui32User1;
    uint8_t pui8MACArray[8];

    //
    // Make sure the main oscillator is enabled because this is required by
    // the PHY.  The system must have a 25MHz crystal attached to the OSC
    // pins. The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
    // frequency is 10MHz or higher.
    //

/*
    //
    // Run from the PLL at 120 MHz.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);

    //
    // Configure the device pins.
    //
    PinoutSet(true, false);

    //
    // Configure UART.
    //
    UARTStdioConfig(0, 115200, g_ui32SysClock);
*/
    //
    // Clear the terminal and print banner.
    //
    UARTprintf("\033[2J\033[H");
    UARTprintf("Ethernet lwIP example\n\n");

    //
    // Configure Port N1 for as an output for the animation LED.
    //
    //MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

    //
    // Initialize LED to OFF (0)
    //
    //MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~GPIO_PIN_1);

    //
    // Configure SysTick for a periodic interrupt.
    //
    //MAP_SysTickPeriodSet(g_ui32SysClock / SYSTICKHZ);
    //MAP_SysTickEnable();
    //MAP_SysTickIntEnable();

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    //
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        // Let the user know there is no MAC address
        //
        UARTprintf("No MAC programmed!\n");
        while(1)
        {
        }
    }

    //
    // Tell the user what we are doing just now.
    //
    UARTprintf("Waiting for IP.\n");

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    //
    // Initialize the lwIP library, using DHCP.
    //
    lwIPInit(g_ui32SysClock, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);

    //
    // Setup the device locator service.
    //
    LocatorInit();
    LocatorMACAddrSet(pui8MACArray);
    LocatorAppTitleSet("EK-TM4C1294XL enet_io");

    logmsg = (LogMsg *)mem_malloc(sizeof(LogMsg));
    if(logmsg == NULL)
    {
        UARTprintf("Malloc Failed\n");
        return 0;
    }
    else
        memset(logmsg, (int)'\0',sizeof(LogMsg));

    echo_init();

    //
    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    //
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    //MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

    //
    // Loop forever, processing the LED blinking.  All the work is done in
    // interrupt handlers.echo_s
    //
    /*while(1)
    {
        //
        // Wait till the SysTick Interrupt indicates to change the state of the
        // LED.
        //
        while(g_bLED == false)
        {
        }
        //
        // Clear the flag.
        //
        g_bLED = false;
        //
        // Toggle the LED.
        //
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,
                         (MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1) ^
                          GPIO_PIN_1));
     }
    return 0;*/
}
