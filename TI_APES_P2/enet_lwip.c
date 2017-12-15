/*****************************************************************************
/ enet_lwip.c - This file has been built over the sample code for WebServer Application using lwIP from TIVA ware software examples.
/*****************************************************************************/

#include "main.h"
#include <enet_lwip.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define PORTNO 5000

#if LWIP_TCP

//global struct for tcp socket
static struct tcp_pcb *echo_pcb;

//log message queue handle
extern xSemaphoreHandle xlogQ_mutex;
extern QueueHandle_t xlogQ;

//common log msg struct
LogMsg logmsg;

//defines for setting system clock
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)

//Interrupt priority definitions.  The top 3 bits of these values are significant with lower values
//indicating higher priority interrupts.
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0

//current ip address to be updated through dhcp
uint32_t g_ui32IPAddress;


/* Function Name - DisplayIPAddress(uint32_t ui32Addr)
 * Function Description - display lwip IP address
 * */
void DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    // Convert the IP Address into a string.
    usprintf(pcBuf, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    // Display the string.
    UARTprintf(pcBuf);
}

/* Function Name - lwIPHostTimerHandler(void)
 * Function Description - Required by lwIP library to support any host-related timer functions.
 * */
void lwIPHostTimerHandler(void)
{
    uint32_t ui32NewIPAddress;

    // Get the current IP address.
    ui32NewIPAddress = lwIPLocalIPAddrGet();

    // See if the IP address has changed.
    if(ui32NewIPAddress != g_ui32IPAddress)
    {
        // See if there is an IP address assigned.
        if(ui32NewIPAddress == 0xffffffff)
        {
            // Indicate that there is no link.
            UARTprintf("Waiting for link.\n");
        }
        else if(ui32NewIPAddress == 0)
        {
            // There is no IP address, so indicate that the DHCP process is
            // running.
            UARTprintf("Waiting for IP address.\n");
        }
        else
        {
            // Display the new IP address.
            UARTprintf("IP Address: ");
            DisplayIPAddress(ui32NewIPAddress);
            UARTprintf("\nOpen a browser and enter the IP address.\n");
        }

        // Save the new IP address.
        g_ui32IPAddress = ui32NewIPAddress;
    }

    // If there is not an IP address.
    if((ui32NewIPAddress == 0) || (ui32NewIPAddress == 0xffffffff))
    {
        // Do nothing and keep waiting.
    }
}

/* Function Name - SysTickIntHandler(void)
 * Function Description - The interrupt handler for the SysTick interrupt.
 * */
void SysTickIntHandler(void)
{
    // Call the lwIP timer handler.
    lwIPTimer(SYSTICKMS);
}

/* Function Name - echo_init(void)
 * Function Description - create tcp socket, bind the socket to PORTNO for listening, listen, then accept any incoming connection
 * */
void echo_init(void)
{
  echo_pcb = tcp_new();
  if (echo_pcb != NULL)
  {
    err_t err;

    err = tcp_bind(echo_pcb, IP_ADDR_ANY, PORTNO); // port number for listening
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

/* Function Name - echo_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
 * Function Description - accept any incoming connection and initialise all the tcp callbacks, also initialise state variable
 *
 * */
err_t echo_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  err_t ret_err;
  struct echo_state *es;

  /* set priority */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  es = (struct echo_state *)mem_malloc(sizeof(struct echo_state));
  if (es != NULL)
  {
    es->state = ES_ACCEPTED;
    es->pcb = newpcb;
    es->retries = 0;
    es->p = NULL;

    /* pass newly allocated state struct to our callbacks */
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

/* Function Name - echo_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
 * Function Description - After a connection has been setup, first send a dummy ACK, and change the state struct to received.
 *                        After state change to receive every incoming packet will be handled by this receive callback
 * */
err_t echo_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct echo_state *es;
  err_t ret_err;
  time_t timeVal;

  es = (struct echo_state *)arg;
  if (p == NULL)
  {
    es->state = ES_CLOSING;
    if(es->p == NULL)
    {
       echo_close(tpcb, es);
    }
    ret_err = ERR_OK;
  }
  else if(err != ERR_OK)
  {
    if (p != NULL)
    {
      es->p = NULL;
      pbuf_free(p);
    }
    ret_err = err;
  }
  else if(es->state == ES_ACCEPTED)
  {

    es->state = ES_RECEIVED;
    UARTprintf("\r\n Accept Successful, %s payload size = %d\r\n",p->payload,p->tot_len);

    //make the global tcp struct payload point to this incoming packet
    es->p = p;

    memset(&logmsg,(int)'\0',sizeof(LogMsg));

    /*dummy request created for sending ACK*/
    logmsg.sourceId= LOGGER_TASK;
    logmsg.requestID= LOG_DATA;
    logmsg.data = 25.0000;
    logmsg.level = INFO;
    strcpy(logmsg.payload,"ACK");
    timeVal = time(NULL);
    strcpy(logmsg.timestamp,ctime(&timeVal));
    p->payload = (void *)&logmsg;
    UARTprintf("\r\n[TIVA] source ID: %d \n", logmsg.sourceId);
    UARTprintf("\r\n[TIVA] Log Level: %d \n", logmsg.level);
    UARTprintf("\r\n[TIVA] Payload: %s   \n", logmsg.payload);
    UARTprintf("\r\n[TIVA] Timestamp: %s \n", logmsg.timestamp);

    tcp_write(tpcb, &logmsg,sizeof(LogMsg),1);

    ret_err = ERR_OK;
  }
  else if(es->state == ES_CLOSING)
  {
    UARTprintf("\r\n Inside Closing = %d ",p->tot_len);
    es->p = NULL;
    pbuf_free(p);
    ret_err = ERR_OK;
  }
  else if (es->state == ES_RECEIVED)
  {
        /* read data */
        if(es->p == NULL)
        {
          es->p = p;
        }
        else
        {
            while(uxQueueSpacesAvailable(xlogQ) != QLEN)
             {
                        if(xSemaphoreTake(xlogQ_mutex,portMAX_DELAY) == pdTRUE)
                        {
                                if(xQueueReceive(xlogQ,&logmsg,portMAX_DELAY) != pdPASS)
                                {
                                  UARTprintf("\r\nQueue is empty task blocks for port max delay\r\n");
                                }
                                else
                                {
                                  UARTprintf("\r\nMsg ID = %d\r\n",logmsg.sourceId);
                                  UARTprintf("\r\nTimestamp = %s\r\n",logmsg.timestamp);
                                  UARTprintf("\r\nLog Data = %s\r\n",logmsg.payload);
                                  p->payload = (void *)&logmsg;
                                 //echo_send(tpcb, es);
                                 tcp_write(tpcb,&logmsg,sizeof(LogMsg),1);
                                 tcp_output(tpcb);

                                }
                        }
                        xSemaphoreGive(xlogQ_mutex);
                 }
        }
        ret_err = ERR_OK;
  }
  else
  {
      UARTprintf("\r\n inside else state %d and state is %d",p->tot_len,es->state);
      es->p = NULL;
      pbuf_free(p);
      ret_err = ERR_OK;
  }
  return ret_err;
}

/* Function Name - echo_error(void *arg, err_t err)
 * Function Description - goes inside this callback to show any error condition, so in case of error just free to allocated memory
 * */
void echo_error(void *arg, err_t err)
{
  struct echo_state *es;
  es = (struct echo_state *)arg;
  if (es != NULL)
  {
    mem_free(es);
  }
}

/* Function Name - echo_poll(void *arg, struct tcp_pcb *tpcb)
 * Function Description - polling on the socket to check if there is any more incoming data,
 *                        if yes, then go back to executing receive callback
 * */
err_t echo_poll(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  struct echo_state *es;

  es = (struct echo_state *)arg;
  if (es != NULL)
  {
    if (es->p != NULL)
    {
        UARTprintf("\r\n check if socket has more data %s ");
    }
    ret_err = ERR_OK;
  }
  else
  {
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/* Function Name - echo_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
 * Function Description - callback called when actually initiating connection from tIVA side
 * */
err_t echo_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  return ERR_OK;
}

/* Function Name - echo_close(struct tcp_pcb *tpcb, struct echo_state *es)
 * Function Description - callback called when closing the socket, set all callback arguments to null to stop tcp transaction,
 *                        then call tcp_close to close socket
 * */
void echo_close(struct tcp_pcb *tpcb, struct echo_state *es)
{
  tcp_arg(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);

  if (es != NULL)
  {
    mem_free(es);
  }
  tcp_close(tpcb);
}

#endif /* LWIP_TCP */

int socketInit(void)
{
    uint32_t ui32User0, ui32User1;
    uint8_t pui8MACArray[8];

    // Clear the terminal and print banner.
    UARTprintf("\033[2J\033[H");
    UARTprintf("Ethernet lwIP example\n\n");

    // Configure the hardware MAC address for Ethernet Controller filtering of incoming packets.
    // The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        // Let the user know there is no MAC address
        UARTprintf("No MAC programmed!\n");
        while(1)
        {
        }
    }

    // waiting for IP
    UARTprintf("Waiting for IP.\n");

    //Program the MAC address into the Ethernet Controller registers.
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    // Initialize the lwIP to use DHCP mode
    lwIPInit(g_ui32SysClock, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);

    LocatorInit();
    LocatorMACAddrSet(pui8MACArray);
    LocatorAppTitleSet("TIVA sockets");

    // we have an IP now initialise all tcp socket functions
    echo_init();

    //setup ethernet interrupts priority to be low, as we do not want ethernet activities to override other system activties like
    // timer interrupts
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);

    return 0;
}
