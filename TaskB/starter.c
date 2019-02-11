/******************************************************************************
*
*   Copyright (C) 2014 Texas Instruments Incorporated
*
*   All rights reserved. Property of Texas Instruments Incorporated.
*   Restricted rights to use, duplicate or disclose this code are
*   granted through contract.
*
*   The program may not be used without the written permission of
*   Texas Instruments Incorporated or against the terms and conditions
*   stipulated in the agreement under which this program has been supplied,
*   and under no circumstances can it be used with non-TI connectivity device.
*
******************************************************************************/

#include "simplelink.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "application_commands.h"
#include "ADCSWTrigger.h"
#include "LaunchPadSwitches.h"
//#include "ST7735.h"

//*****************************************************************************
//! \addtogroup example_list
//! <h1>CC3100 Starter Example</h1>
//!
//! <!--##### README BEGIN #####-->
//! This example is used as a base to build on for other examples for the 
//! CC3100
//! <!--##### README END #####-->
//
//*****************************************************************************

#define SSID_NAME  "Pixel2xl"          // Open AP name to connect to.
#define ATYPE 'a'            // used by client to tag USP packet
#define BTYPE 'b'            // used by RAMP client to tag USP packet
/* IP addressed of server side socket. Should be in long format, E.g: 0xc0a80164 == 192.168.0.101 */
#define IP_ADDR         0xc0a82bfb   // 192=0xC0, 168=0xa8, 0x00=0, 0x65 = 101
#define PORT_NUM        5001         // Port number to be used 
#define BUF_SIZE        12
#define SENSORNODE 0       // true if this node is sending UDP packets, using client
#define DISPLAYNODE 1      // true if this node is receiving UDP packets, using server
#define CRTNODE 0          // true if this node is runs an intepreter on UART0
#define EKG 0              // client simulates ekg instead of measuring ADC
#define RAMP 0             // client sends ramp data instead of measuring ADC
#define ADC 1              // client gets data from ADC, Ain7 = PD0
/*enum
{
    CONNECTED = 0x1,
    IP_ACQUIRED = 0x2
}e_Stauts;*/

UINT8 uBuf[BUF_SIZE];  // payload


#define UNUSED(x) (x = x)

#define PING_INTERVAL     1000
#define PING_TIMEOUT      3000
#define PING_SIZE         20
#define NO_OF_ATTEMPTS    3

#define CONNECTION_STATUS_BIT   0
#define IP_AQUIRED_STATUS_BIT   1
#define IP_LEASED_STATUS_BIT    2
#define PING_DONE_STATUS_BIT    3
typedef enum{
    CONNECTED = 0x01,
    IP_AQUIRED = 0x02,
    IP_LEASED = 0x04,
    PING_DONE = 0x08

}e_Status;
UINT8 g_Status = 0;

unsigned int g_PingPacketsRecv = 0;
/* CC3100 booster pack connections (unused pins can be used by user application)
Pin  Signal        Direction      Pin   Signal     Direction
P1.1  3.3 VCC         IN          P2.1  Gnd   GND      IN
P1.2  PB5 UNUSED      NA          P2.2  PB2   IRQ      OUT
P1.3  PB0 UART1_TX    OUT         P2.3  PE0   SPI_CS   IN
P1.4  PB1 UART1_RX    IN          P2.4  PF0   UNUSED   NA
P1.5  PE4 nHIB        IN          P2.5  Reset nRESET   IN
P1.6  PE5 UNUSED      NA          P2.6  PB7   SPI_MOSI IN
P1.7  PB4 SPI_CLK     IN          P2.7  PB6   SPI_MISO OUT
P1.8  PA5 UNUSED      NA          P2.8  PA4   UNUSED   NA
P1.9  PA6 UNUSED      NA          P2.9  PA3   UNUSED   NA
P1.10 PA7 UNUSED      NA          P2.10 PA2   UNUSED   NA

Pin  Signal        Direction      Pin   Signal      Direction
P3.1  +5  +5 V       IN           P4.1  PF2 UNUSED      OUT
P3.2  Gnd GND        IN           P4.2  PF3 UNUSED      OUT
P3.3  PD0 UNUSED     NA           P4.3  PB3 UNUSED      NA
P3.4  PD1 UNUSED     NA           P4.4  PC4 UART1_CTS   IN
P3.5  PD2 UNUSED     NA           P4.5  PC5 UART1_RTS   OUT
P3.6  PD3 UNUSED     NA           P4.6  PC6 UNUSED      NA
P3.7  PE1 UNUSED     NA           P4.7  PC7 NWP_LOG_TX  OUT
P3.8  PE2 UNUSED     NA           P4.8  PD6 WLAN_LOG_TX OUT
P3.9  PE3 UNUSED     NA           P4.9  PD7 UNUSED      IN (see R74)
P3.10 PF1 UNUSED     NA           P4.10 PF4 UNUSED      OUT(see R75)

Sensor input on Ain7 = PD0
*/
#if EKG
/* http://www.physionet.org/cgi-bin/rdsamp?database=aami-ec13
   http://physionet.caregroup.harvard.edu/physiobank/database/aami-ec13/
   ECG signal sampled at 100 Hz with 12-bit resolution
*/
uint32_t EKGindex=0;
#define EKGSIZE 340
uint16_t const EKGbuf[EKGSIZE]={
 1987, 1981, 1987, 1971, 1981, 1976, 1976, 1956, 1966, 1951, 1966, 1951, 1966, 1951, 1961, 1956,
 1956, 1946, 1961, 1940, 1956, 2002, 2053, 2063, 2094, 2207, 2191, 2094, 2053, 2007, 1961, 1843,
 1848, 1843, 1843, 1802, 1915, 2396, 3231, 3553, 1889, 1203, 1444, 1690, 1889, 1874, 1905, 1889, 
 1930, 1920, 1956, 1951, 1981, 1992, 2033, 2043, 2084, 2094, 2140, 2161, 2202, 2232, 2258, 2278, 
 2284, 2273, 2289, 2284, 2253, 2222, 2171, 2125, 2089, 2068, 2053, 2043, 2043, 2038, 2033, 2038,
 2048, 2043, 2028, 2028, 2033, 2022, 2012, 2002, 2002, 1987, 1981, 1976, 1992, 1987, 1966, 1971,
 1956, 1981, 1971, 1966, 1966, 1961, 1956, 1971, 1961, 1951, 1940, 1956, 1951, 1976, 2002, 2053,
 2079, 2120, 2248, 2196, 2099, 2033, 2022, 1930, 1864, 1853, 1859, 1859, 1828, 2058, 2637, 3261,
 3031, 1884, 1290, 1393, 1690, 1869, 1900, 1894, 1900, 1940, 1925, 1956, 1961, 1976, 2007, 2028, 
 2053, 2068, 2094, 2130, 2166, 2191, 2217, 2263, 2243, 2273, 2263, 2268, 2268, 2212, 2186, 2150, 
 2099, 2084, 2043, 2053, 2043, 2033, 2028, 2038, 2048, 2028, 2043, 2007, 2012, 1997, 2007, 1992, 
 1971, 1997, 1971, 1956, 1966, 1956, 1966, 1971, 1961, 1961, 1951, 1951, 1940, 1966, 1930, 1946, 
 1951, 1946, 1971, 2043, 2058, 2104, 2237, 2196, 2125, 2043, 2022, 1935, 1869, 1864, 1823, 1869, 
 1802, 2017, 2591, 3287, 3297, 1940, 1234, 1341, 1664, 1879, 1889, 1910, 1894, 1925, 1925, 1956, 
 1951, 1997, 1997, 2038, 2043, 2079, 2130, 2125, 2196, 2207, 2253, 2253, 2294, 2289, 2299, 2299, 
 2268, 2232, 2202, 2140, 2084, 2074, 2048, 2033, 2017, 2048, 2028, 2043, 2022, 2017, 2033, 2038, 
 2012, 2007, 1997, 2002, 2007, 1971, 1981, 1951, 1961, 1961, 1956, 1946, 1946, 1966, 1940, 1940, 
 1946, 1940, 1951, 1946, 1956, 2012, 2058, 2084, 2120, 2243, 2176, 2094, 2012, 1987, 1889, 1833, 
 1818, 1843, 1828, 1777, 2120, 2934, 3579, 2688, 1597, 1213, 1434, 1731, 1869, 1864, 1879, 1910, 
 1910, 1930, 1940, 1961, 1987, 2017, 2028, 2063, 2099, 2120, 2156, 2217, 2253, 2258, 2289, 2289, 
 2319, 2314, 2289, 2268, 2222, 2176, 2140, 2074, 2033, 2017, 2012, 2007, 2007, 2012, 2007, 2028, 
 2007, 2007, 2017, 2002  };
#endif  

//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
#if CRTNODE
static char g_cInput[MAX_COMMAND_SIZE];
#endif
//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// Table of valid command strings, callback functions and help messages.  This
// is used by the cmdline module.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    {"help",          CMD_help,
                  " : Display this list of commands." },
//    {"smartconfig",   CMD_smartConfig,
//                  " : First time simple configuration. Use app on smartphone\n"
//"                     to connect to network." },
//    {"connect",       CMD_connect,
//                  " : [1]SSID : Connect to an open access point." },
//    {"ipconfig",      CMD_ipConfig,
//                  " : [1]Local IP address [2]Default gateway\n"
//"                     [3](optional) Network mask. For DHCP give no arguments."},
//    {"socketopen",    CMD_socketOpen,
//                  " : [1]UDP/TCP : Open socket, specify TCP or UDP." },
//    {"bind",          CMD_bind,
//                  " : [1]Port to bind socket to" },
//    {"senddata",      CMD_sendData,
//                  " : [1]IP Address [2]Destination Port\n"
//"                     [3]Data to send ( < 255 bytes, no spaces allowed)." },
//    {"receivedata",   CMD_receiveData,
//                  " : Receive data on a socket." },
//    {"mdnsadvertise", CMD_mdnsadvertise,
//                  " : [1](optional) name to broadcast via mDNS to connected\n"
//"                     network." },
//    {"resetcc3000",   CMD_cc3000reset,
//                  " : Reset the CC3000."},
//    {"socketclose",   CMD_socketClose,
//                  " : Close the open socket." },
//    {"disconnect",    CMD_disconnect,
//                  " : Disconnect from the network." },
//    {"deletepolicy",  CMD_deletePolicy,
//                  " : Delete the automatic connection policy. On reset CC3000\n"
//"                     will not automatically reconnect to the network." },
    {"ping",          CMD_ping,
                  " : ping the gateway, usually 192.168.1.1"},
    { 0, 0, 0 }
};

//*****************************************************************************
//    \brief This function handles WLAN events
//
//    \param[in]      pWlanEvents is the event passed to the handler
//
//    \return         None
//
//    \note
//
//    \warning
//******************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvents)
{
    switch(pWlanEvents->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
            g_Status |= CONNECTED;
            UARTprintf("    Wlan Connected\n");
        break;

        case SL_WLAN_DISCONNECT_EVENT:
            g_Status &= ~(1 << CONNECTION_STATUS_BIT | 1 << IP_AQUIRED_STATUS_BIT);
            UARTprintf("    Wlan Disconnect\n");
        break;

        case SL_WLAN_SMART_CONFIG_START_EVENT:
            UARTprintf("    Wlan SmartConfig Start\n");
        break;

        case SL_WLAN_SMART_CONFIG_STOP_EVENT:
            UARTprintf("    Wlan SmartConfig Stop\n");
        break;

        case SL_WLAN_STA_CONNECTED_EVENT:
            UARTprintf("    SL_WLAN_STA_CONNECTED_EVENT\n");
        break;
    
        case SL_WLAN_STA_DISCONNECTED_EVENT:
            UARTprintf("    SL_WLAN_STA_DISCONNECTED_EVENT\n");
        break;

        default:
            UARTprintf("    WlanEventHandler, unknown event detected: %d\n", ((SlWlanEvent_t*)pWlanEvents)->Event );
        break;
    }
}

//******************************************************************************
//    \brief This function handles events for IP address acquisition via DHCP
//           indication
//
//    \param[in]      pNetAppEvent is the event passed to the handler
//
//    \return         None
//
//    \note
//
//    \warning
//******************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    switch( pNetAppEvent->Event )
    {
        case SL_NETAPP_IPV4_ACQUIRED:
            g_Status |= IP_AQUIRED;
            UARTprintf("    SL_NETAPP_IPV4_ACQUIRED\n");
        break;

        case SL_NETAPP_IPV6_ACQUIRED:
            UARTprintf("    SL_NETAPP_IPV6_ACQUIRED\n");
        break;

        case SL_NETAPP_SOCKET_TX_FAILED:
            UARTprintf("    SL_NETAPP_SOCKET_TX_FAILED\n");
        break;

        case SL_NETAPP_IP_LEASED:
            UARTprintf("    SL_NETAPP_IP_LEASED\n");
        break;

        case SL_NETAPP_IP_RELEASED:
            UARTprintf("    SL_NETAPP_IP_RELEASED\n");
        break;

        default:
            UARTprintf("    NetApp Event Handler: unknown handle: %d\n",pNetAppEvent->Event);
        break;
    }
}

//******************************************************************************
//    \brief This function handles callback for the HTTP server events
//
//    \param[in]      pServerEvent - Contains the relevant event information
//    \param[in]      pServerResponse - Should be filled by the user with the
//                    relevant response information
//
//    \return         None
//
//    \note
//
//    \warning
//******************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    /* Unused in this application */
    UARTprintf("    SimpleLink HTTP Server Callback\n");
}


//******************************************************************************
//    \brief This function handles ping report events
//
//    \param[in]      pPingReport holds the ping report statistics
//
//    \return         None
//
//    \note
//
//    \warning
//******************************************************************************
static void SimpleLinkPingReport(SlPingReport_t *pPingReport)
{
    g_Status |= PING_DONE;
    g_PingPacketsRecv = pPingReport->PacketsReceived;
    UARTprintf("Ping Report Callback:\n");
    UARTprintf("     PacketsSent: %d\n    PacketsReceived: %d\n    MinRoundTime: %dms\n    MaxRoundTime: %dms\n    AvgRoundTime: %dms\n    TestTime: %dms\n"
        , pPingReport->PacketsSent, pPingReport->PacketsReceived, pPingReport->MinRoundTime, pPingReport->MaxRoundTime, pPingReport->AvgRoundTime, pPingReport->TestTime);
}


//******************************************************************************
//    \brief Connecting to a WLAN Access point
//
//    This function connects to the required AP (SSID_NAME).
//    This code example assumes the AP doesn't use WIFI security.
//    The function will return once we are connected and have acquired IP address
//
//    \param[in]  None
//
//    \return     None
//
//    \note
//
//    \warning    If the WLAN connection fails or we don't aquire an IP address,
//                We will be stuck in this function forever.
//******************************************************************************
void WlanConnect()
{
    SlSecParams_t secParams;

    secParams.Key = "";
    secParams.KeyLen = 0;
    secParams.Type = SL_SEC_TYPE_OPEN;

    sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);

    while((0 == (g_Status & CONNECTED)) || (0 == (g_Status & IP_AQUIRED)))
    {
        _SlNonOsMainLoopTask();
    }
}
//*****************************************************************************
//
// Print the help strings for all commands.
//
//*****************************************************************************
int
CMD_help(int argc, char **argv)
{
    int32_t i32Index;

    (void)argc;
    (void)argv;

    //
    // Start at the beginning of the command table.
    //
    i32Index = 0;

    //
    // Get to the start of a clean line on the serial output.
    //
    UARTprintf("\nAvailable Commands\n------------------\n\n");

    //
    // Display strings until we run out of them.
    //
    while(g_psCmdTable[i32Index].pcCmd)
    {
        //
        // Display help information for a single command.
        //
        UARTprintf("%17s %s\n", g_psCmdTable[i32Index].pcCmd,
                   g_psCmdTable[i32Index].pcHelp);
        i32Index++;

        //
        // Make sure we've sent all the UART data before we add more to the
        // buffer.
        //
        UARTFlushTx(0);
    }

    //
    // Leave a blank line after the help strings.
    //
    UARTprintf("\n");

    return(0);
}

//*****************************************************************************
//
// Ping an IP Address
// Arguments:
//  [1] IP address to ping
//  [2] (optional) max number of tries
//  [3] (optional) timeout in milliseconds
//
//*****************************************************************************
int
CMD_ping(int argc, char **argv)
{
    SlPingStartCommand_t PingParams;
    SlPingReport_t Report;
    _NetCfgIpV4Args_t ipV4;

    unsigned char len = sizeof(_NetCfgIpV4Args_t);;
    unsigned char IsDHCP = 0;

    /* Read the IP parameter */
    sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&IsDHCP,&len,
            (unsigned char *)&ipV4);

    /* Set the ping parameters */
    PingParams.PingIntervalTime = PING_INTERVAL;
    PingParams.PingSize = PING_SIZE;
    PingParams.PingRequestTimeout = PING_TIMEOUT;
    PingParams.TotalNumberOfAttempts = NO_OF_ATTEMPTS;
    PingParams.Flags = 0;
    PingParams.Ip = ipV4.ipV4Gateway; /* Fill the GW IP address, which is our AP address */

    /* Check for LAN connection */
    UARTprintf("Pinging the Gateway...\n");
    sl_NetAppPingStart((SlPingStartCommand_t*)&PingParams, SL_AF_INET,
                       (SlPingReport_t*)&Report, SimpleLinkPingReport);

    while(0 == (g_Status & PING_DONE))
    {
        _SlNonOsMainLoopTask();
    }

    g_Status &= ~(1 << PING_DONE_STATUS_BIT);
    UARTprintf("Ping Done\n");
    if (!g_PingPacketsRecv)
    {
        /* Problem with LAN connection */
        UARTprintf("Problem with the LAN connection\n");
        return -1;
    }
  return 0;
}

/*!
    \brief Sends UDP packets to Server IP_ADDR
    Opening a UDP client side socket and sending data
    This function opens a UDP socket and tries to connect to a Server IP_ADDR
    waiting on port PORT_NUM.
    Then the function will send 2 UDP packets to the server about .

    \param[in]      port number on which the server will be listening on

    \return         0 on success, -1 on Error.

    \note

    \warning
*/
#if SENSORNODE
int main0(void){  
  UINT8             IsDHCP = 0;
  _NetCfgIpV4Args_t ipV4;
  SlSockAddrIn_t    Addr;
  UINT16            AddrSize = 0;
  INT16             SockID = 0;
  UINT32            data;
  unsigned char     len = sizeof(_NetCfgIpV4Args_t);
  initClk();         // PLL 50 MHz, ADC needs PPL active          15
  ADC0_InitSWTriggerSeq3(7);  // Ain7 is on PD0                   16
  sl_Start(0, 0, 0); // Initializing the CC3100 device            17
  WlanConnect();     // connect to AP                             18
  sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&IsDHCP,&len,       // 19
               (unsigned char *)&ipV4);                        // 20
  Addr.sin_family = SL_AF_INET;                       //          21 
  Addr.sin_port = sl_Htons((UINT16)PORT_NUM);         //          22
  Addr.sin_addr.s_addr = sl_Htonl((UINT32)IP_ADDR);   //          23
  AddrSize = sizeof(SlSockAddrIn_t);                  //          24
  SockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);    //          25
  while(1){
    uBuf[0] = ATYPE;      // analog data type                     26
    uBuf[1] = '=';        //                                      27
    data = ADC0_InSeq3(); // 0 to 4095, Ain7 is on PD0            28
    Int2Str(data,(char*)&uBuf[2]); // 6 digit number              29
		//UARTprintf((char*)&uBuf[2]);
    sl_SendTo(SockID, uBuf, BUF_SIZE, 0,        //                30
                         (SlSockAddr_t *)&Addr, AddrSize); //     31
    ROM_SysCtlDelay(ROM_SysCtlClockGet() / 25);  // 40ms          32
  }
}


int main(void){
  UINT8             IsDHCP = 0;
  _NetCfgIpV4Args_t ipV4;
  SlSockAddrIn_t    Addr;
  UINT16            AddrSize = 0;
  INT16             SockID = 0;
  INT16             Status = 1;  // ok
  UINT32            data;
  unsigned char     len = sizeof(_NetCfgIpV4Args_t);
  stopWDT();        // Stop WDT 
  initClk();        // PLL 50 MHz, ADC needs PPL active
  Board_Init();     // initialize LaunchPad I/O 
  ConfigureUART();  // Initialize the UART.
  UARTprintf("Section 11.4 IoT example, Volume 2 Real-time interfacing\n");
#if ADC
  ADC0_InitSWTriggerSeq3(7);  // Ain7 is on PD0
  UARTprintf("This node is configured to measure signals from Ain7=PD0\n");
#endif
#if EKG
  UARTprintf("This node is configured to generate simulated EKG data\n");
#endif
#if RAMP
  UARTprintf("This node is configured to send 0,1,2...,4095,0,1,...\n");
#endif
  UARTprintf("  and send UDP packets to IP: %d.%d.%d.%d  Port: %d\n\n",
      SL_IPV4_BYTE(IP_ADDR,3), SL_IPV4_BYTE(IP_ADDR,2), 
      SL_IPV4_BYTE(IP_ADDR,1), SL_IPV4_BYTE(IP_ADDR,0),PORT_NUM);
  sl_Start(0, 0, 0);/* Initializing the CC3100 device */
  while(1){
    /* Connecting to WLAN AP - Set with static parameters defined at the top
       After this call we will be connected and have IP address */
    WlanConnect();   // connect to AP
    /* Read the IP parameter */
    sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&IsDHCP,&len,(unsigned char *)&ipV4);
    UARTprintf("This node is at IP: %d.%d.%d.%d\n", SL_IPV4_BYTE(ipV4.ipV4,3), SL_IPV4_BYTE(ipV4.ipV4,2), SL_IPV4_BYTE(ipV4.ipV4,1), SL_IPV4_BYTE(ipV4.ipV4,0));
    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons((UINT16)PORT_NUM);
    Addr.sin_addr.s_addr = sl_Htonl((UINT32)IP_ADDR);
    AddrSize = sizeof(SlSockAddrIn_t);
    SockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    if( SockID < 0 ){
      UARTprintf("SockIDerror ");
      Status = -1; // error
    }else{
      Status = 1; // socket ok
    }
    while(Status > 0){
      UARTprintf("\nSending a UDP packet ...");
      uBuf[0] = ATYPE;   // defines this as an analog data type
      uBuf[1] = '='; 
#if ADC
      uBuf[0] = ATYPE;   // defines this as an analog data type
      data = ADC0_InSeq3(); // 0 to 4095, Ain7 is on PD0
			
#endif
#if EKG
      uBuf[0] = ATYPE;   // defines this as an analog data type
      data = EKGbuf[EKGindex];
      EKGindex = (EKGindex+1)%EKGSIZE; // 100 Hz
#endif
#if RAMP
      uBuf[0] = BTYPE;   // defines this as a ramp data type
      data = (data+1)&0x0FFF; // 0 to 4095
#endif
			
      Int2Str(data,(char*)&uBuf[2]); // [2] to [7] is 6 digit number
      UARTprintf(" %s ",uBuf);
      LED_Toggle();
      Status = sl_SendTo(SockID, uBuf, BUF_SIZE, 0,
                       (SlSockAddr_t *)&Addr, AddrSize);
      ROM_SysCtlDelay(ROM_SysCtlClockGet() / 25); // 80ms
      if( Status <= 0 ){
        UARTprintf("SockIDerror %d ",Status);
        sl_Close(SockID);
        SockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
        if( SockID < 0 ){
          UARTprintf("SockIDerror ");
          Status = -1; // error
        }else{
          Status = 1; // socket ok
        }
      }else{
       UARTprintf("ok");
      }     
    }
  }
}
#endif

/*!
    \brief Opening a UDP server side socket and receiving data

    This function opens a UDP socket in Listen mode and waits for an incoming
    UDP connection. If a socket connection is established then the function
    will try to read 1000 UDP packets from the connected client.

    \param[in]      port number on which the server will be listening on

    \return         0 on success, Negative value on Error.

    \note

    \warning
*/
#if DISPLAYNODE
int main3(void){
  UINT8             IsDHCP = 0;
  _NetCfgIpV4Args_t ipV4;
  SlSockAddrIn_t    Addr, LocalAddr;
  UINT16            AddrSize = 0;
  INT16             SockID = 0;
  INT16             Status = 1;  // ok
  UINT32            data;
  unsigned char     len = sizeof(_NetCfgIpV4Args_t);
  initClk();        // PLL 50 MHz, ADC needs PPL active           16
  //ST7735_InitR(INITR_REDTAB);                  // Initialize      17
  //ST7735_OutString("Internet of Things\n");    //                 18
  //ST7735_OutString("Embedded Systems\n");      //                 19
  //ST7735_OutString("WiFi Communication");         //                 20
  //ST7735_PlotClear(0,4095);  // range from 0 to 4095              21
  sl_Start(0, 0, 0); // Initializing the CC3100 device            22
  WlanConnect();     // connect to AP                             23
  sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&IsDHCP,&len,   //     24
               (unsigned char *)&ipV4);                    //     25
  LocalAddr.sin_family = SL_AF_INET;                       //     26
  LocalAddr.sin_port = sl_Htons((UINT16)PORT_NUM);         //     27
  LocalAddr.sin_addr.s_addr = 0;                           //     28
  AddrSize = sizeof(SlSockAddrIn_t);                       //     29
  while(1){
    SockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);       //     31   
    Status = sl_Bind(SockID, (SlSockAddr_t *)&LocalAddr,   //     32
                       AddrSize);                          //     33
    Status = sl_RecvFrom(SockID, uBuf, BUF_SIZE, 0,        //     34
          (SlSockAddr_t *)&Addr, (SlSocklen_t*)&AddrSize );//     35
    if((uBuf[0]==ATYPE)&&(uBuf[1]== '=')){                 //     36
      int i,bOk; uint32_t place;                           //     37
      data = 0; bOk = 1;                                   //     38
      i=4;  // ignore possible negative sign                      39
      for(place = 1000; place; place = place/10){          //     40
        if((uBuf[i]&0xF0)==0x30){ // ignore spaces                41
          data += place*(uBuf[i]-0x30);                    //     42
        }else{                                             //     43
          if((uBuf[i]&0xF0)!= ' '){                        //     44
            bOk = 0;                                       //     45
          }                                                //     46
        }                                                  //     47
        i++;                                               //     48
      }                                                    //     49
      //if(bOk){                                             //     50
        //ST7735_PlotLine(data);                             //     51
        //ST7735_PlotNextErase();                            //     51
      //}
    }
  }
}


UINT32 ReceiverErrorCnt;
int main(void){
  UINT8             IsDHCP = 0;
  _NetCfgIpV4Args_t ipV4;
  SlSockAddrIn_t    Addr;
  SlSockAddrIn_t    LocalAddr;
  UINT16            AddrSize = 0;
  INT16             SockID = 0;
  INT16             Status = 1;  // ok
  UINT32            data;
  unsigned char     len = sizeof(_NetCfgIpV4Args_t);
  stopWDT();        // Stop WDT 
  initClk();        // PLL 50 MHz, ADC needs PPL active
  Board_Init();     // initialize LaunchPad I/O 
  ConfigureUART();  // Initialize the UART.
  UARTprintf("Section 11.4 IoT example, Volume 2 Real-time interfacing\n");
  UARTprintf("This node is configured to receive UDP packets\n");
  UARTprintf("This node should be at IP: %d.%d.%d.%d  Port: %d\n\n",
      SL_IPV4_BYTE(IP_ADDR,3), SL_IPV4_BYTE(IP_ADDR,2), 
      SL_IPV4_BYTE(IP_ADDR,1), SL_IPV4_BYTE(IP_ADDR,0),PORT_NUM);
  //ST7735_InitR(INITR_REDTAB);
  //ST7735_OutString("Internet of Things\n");
  //ST7735_OutString("Embed Sys\n");
  //ST7735_OutString("IP ");
  //ST7735_PlotClear(0,4095);  // range from 0 to 4095
  ReceiverErrorCnt = 0;
  sl_Start(0, 0, 0);   /* Initializing the CC3100 device */
  while(1){
    /* Connecting to WLAN AP - Set with static parameters defined at the top
       After this call we will be connected and have IP address */
    WlanConnect();   // connect to AP
    /* Read the IP parameter */
    sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&IsDHCP,&len,(unsigned char *)&ipV4);
    UARTprintf("This node is at IP: %d.%d.%d.%d\n", SL_IPV4_BYTE(ipV4.ipV4,3), SL_IPV4_BYTE(ipV4.ipV4,2), SL_IPV4_BYTE(ipV4.ipV4,1), SL_IPV4_BYTE(ipV4.ipV4,0));
    //ST7735_SetCursor(3,2);
      //ST7735_OutUDec(SL_IPV4_BYTE(ipV4.ipV4,3)); ST7735_OutChar('.');  
      //ST7735_OutUDec(SL_IPV4_BYTE(ipV4.ipV4,2)); ST7735_OutChar('.');  
      //ST7735_OutUDec(SL_IPV4_BYTE(ipV4.ipV4,1)); ST7735_OutChar('.');  
      //ST7735_OutUDec(SL_IPV4_BYTE(ipV4.ipV4,0));  
    LocalAddr.sin_family = SL_AF_INET;
    LocalAddr.sin_port = sl_Htons((UINT16)PORT_NUM);
    LocalAddr.sin_addr.s_addr = 0;
    AddrSize = sizeof(SlSockAddrIn_t);
    SockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);     
    if( SockID < 0 ){
      UARTprintf("SockIDerror\n");
      Status = -1; // error
    }else{
      Status = sl_Bind(SockID, (SlSockAddr_t *)&LocalAddr, AddrSize);
      if( Status < 0 ){
        sl_Close(SockID); 
        UARTprintf("Sock Bind error\n");
      }else{
        Status = 1; // bind is ok
      }
    }
    while(Status > 0){
      UARTprintf("\nReceiving a UDP packet ...");

      Status = sl_RecvFrom(SockID, uBuf, BUF_SIZE, 0,
                  (SlSockAddr_t *)&Addr, (SlSocklen_t*)&AddrSize );
      if( Status <= 0 ){
        sl_Close(SockID); ReceiverErrorCnt++;
        UARTprintf("Receive error %d ",Status);
      }else{
        LED_Toggle();
        UARTprintf("%s ",uBuf);
        if(((uBuf[0]==ATYPE)||(uBuf[0]==BTYPE))&&(uBuf[1]== '=')){ int i,bOk; uint32_t place;
          data = 0; bOk = 1;
          i=4;  // ignore possible negative sign
          for(place = 1000; place; place = place/10){
            if((uBuf[i]&0xF0)==0x30){ // ignore spaces
              data += place*(uBuf[i]-0x30);
            }else{
              if((uBuf[i]&0xF0)!= ' '){
                bOk = 0;
              }
            }
            i++;
          }
          //if(bOk){
            //ST7735_PlotLine(data);
            //ST7735_PlotNextErase(); 
          //}
        }
      }
    }
  }
}

#endif
#if CRTNODE
//*****************************************************************************
//
// Main Loop for command line intepreter
//
//*****************************************************************************

int main(void)
{
    UINT8  IsDHCP = 0;
    int32_t i32CommandStatus;
    _NetCfgIpV4Args_t ipV4;

    unsigned char len = sizeof(_NetCfgIpV4Args_t);
    int Status = 0;

    /* Stop WDT */
    stopWDT();

    /* Initialize the system clock of MCU */
    initClk();

    Board_Init();       // initialize LaunchPad I/O and PD1 LED
    ConfigureUART();    // Initialize the UART.
    UARTprintf("Section 11.4 IoT example, Volume 2 Real-time interfacing\n");
    UARTprintf("This application is configured to measure analog signals from Ain7=PD0\n");
    UARTprintf("  and send UDP packets to IP: %d.%d.%d.%d  Port: %d\n\n",
      SL_IPV4_BYTE(IP_ADDR,3), SL_IPV4_BYTE(IP_ADDR,2), 
      SL_IPV4_BYTE(IP_ADDR,1), SL_IPV4_BYTE(IP_ADDR,0),PORT_NUM);
    /* Initializing the CC3100 device */
    sl_Start(0, 0, 0);

    /* Connecting to WLAN AP - Set with static parameters defined at the top
       After this call we will be connected and have IP address */
    WlanConnect();

    /* Read the IP parameter */
    sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO,&IsDHCP,&len,
            (unsigned char *)&ipV4);

    //Print the IP
    UARTprintf("This node is at IP: %d.%d.%d.%d\n",  SL_IPV4_BYTE(ipV4.ipV4,3), SL_IPV4_BYTE(ipV4.ipV4,2), SL_IPV4_BYTE(ipV4.ipV4,1), SL_IPV4_BYTE(ipV4.ipV4,0));

    //
    // Loop forever waiting  for commands from PC...
    //
    while(1)
    {
        //
        // Print prompt for user.
        //
        UARTprintf("\n>");

        //
        // Peek to see if a full command is ready for processing.
        //
        while(UARTPeek('\r') == -1)
        {
            //
            // Approximately 1 millisecond delay.
            //
            ROM_SysCtlDelay(ROM_SysCtlClockGet() / 3000);
        }

        //
        // A '\r' was detected so get the line of text from the receive buffer.
        //
        UARTgets(g_cInput,sizeof(g_cInput));

        //
        // Pass the line from the user to the command processor.
        // It will be parsed and valid commands executed.
        //
        i32CommandStatus = CmdLineProcess(g_cInput);

        //
        // Handle the case of bad command.
        //
        if(i32CommandStatus == CMDLINE_BAD_CMD)
        {
            UARTprintf("    Bad command. Try again.\n");
        }
        //
        // Handle the case of too many arguments.
        //
        else if(i32CommandStatus == CMDLINE_TOO_MANY_ARGS)
        {
            UARTprintf("    Too many arguments for command. Try again.\n");
        }
        //
        // Handle the case of too few arguments.
        //
        else if(i32CommandStatus == CMDLINE_TOO_FEW_ARGS)
        {
            UARTprintf("    Too few arguments for command. Try again.\n");
        }
        //
        // Handle the case of too few arguments.
        //
        else if(i32CommandStatus == CMDLINE_INVALID_ARG)
        {
            UARTprintf("    Invalid command argument(s). Try again.\n");
        }
    }
    
}
#endif
