# CS 795 IoT: WIRELESS CONNECTIVITY

```
Name: Hrishikesh Gadkari
UIN: 01085143
```
Abstract

To design a system with Launchpad Tiva C Series TM4C123GH6PM around an ARM Cortex-M
processor core and CC3100 SimpleLink Wi-Fi Boostpack to connect
to openweathermap.org server and request for weather details of Norfolk city and then with the
Tiva Sensor Hub, implement a communication system between two Tiva boards via an IEEE
802.11 – WiFi module where one Launchpad is used to collect the sensor data and the second to
display the data with the Tiva Sensor Hub on PuTTY.

I. Introduction

The system implementation problem comprised of two tasks where task 1 was an individual task
whereas the Task 2 was to be implemented in a team of two members.

Task 1 was a demonstration of how to connect to openweathermap.org server and request for
weather details of Norfolk city. The application opens a TCP socket with the server and sends a
HTTP Get request to get the weather details. The received data is processed and displayed on the
console. The main.c initializes the device, connects to an AP, opens a TCP socket, requests and
displays the weather information. This application requires an access-point with internet
connectivity and use UART serial connection in order to display information on the PC with
PuTTY.

Task 2 was implemented along with Aniket Chandak as we had to perform the task in a group of
two members where my role was to be a server whereas he was the client. The system uses one
Launchpad Tiva C Series TM4C123GH6PM to collect the light sensor data and the second to
display the data. Data are transmitted across a wireless network using UDP packets. The client
measures the sensing data with Sensor Hub from ADC and sends UDP packets to an AP. The
server receives UDP packets from the AP and displays the data on PuTTY via UART Virtual
COM PORT and plots the data using Python. The CC3100 SimpleLink Wi-Fi Boostpack
implements the Internet stack with a combination of Hardware and Software components.
Software in the Launchpad performs system calls to the CC3100 to affect wireless
communication. We used two Launchpads to develop a solution that transmits UDP packets from
one smart object to another. UDP is simpler than TCP and appropriate for applications requiring
simplicity and speed. Furthermore, to use UDP the application must tolerate lost or out of order
packets. UDP provides a best-effort datagram delivery service.

II. Interface Diagram


Circuit schematics of ARM Cortex M4 GPIO port connections
.

```
Tiva C Series TM4C123GH6PM Launchpad
```

```
CC3100 Interface
```
Sensor Hub Booster Pack


III. SW Development

Task A: Run Get Weather Application

The first step was to create an API Key from openweathermap.org and modify the ‘API Key’
value in main.c file.

Then modify the values of below macros for the device to connect to the Access Point.

And finally defining the id for getting the weather information of Norfolk from the server and
setting units to imperial to get the data in imperial format

```
Client-Server (openweathermap.org) Communication
```

The system is developed using socket connections where the data is fetched in RecvBuff in xml
format.

Then the RecvBuff is parsed to get the city name, current temperature and weather conditions.


The challenges faced were linking all the dependent source and header files first and then
debugging the files for results. Several files had to be modified to include paths and also make
compatible with one another for defining the variables and identifiers,
Also there were problems in parsing the buffer data from the openweathermap server to display
the results in proper format which were later overcome by my coding and debugging skills along
with the instructors help.

Task B: Implement a communication system between two IoT devices (e.g., Tiva boards)
via an IEEE 802.11 **–** WiFi module

The Task was divided between me and Aniket Chandak with me being the server to collect the
sensor data and displaying on Putty via UART virtual COM port whereas Aniket’s role was to
collect the sensor data through Sensor hub and transmitting it to the server.

First, we configured a wireless access point, using our personal smart phone as the access point
by setting the name in starter.c file

Then defining the server’s IP address, port number as well for establishing socket
communication.

Setting up the client:
The client smart object includes a LaunchPad, a CC3100 booster, and sensor interface that
creates an analog input on PD0. Then defining Sensor Node to be true to send the UDP packets
as client using sockets.

Setting up the server:
The server smart object includes a LaunchPad, a CC3100 booster. Then defining the Display
Node to be true to receive packets as server using sockets.

First we tested the system connection by defining RAMP to be true and by sending only the UDP
packets instead of the sensor data.


Server Side

```
Client Side
```

Then for the client side to collect the sensor data through sensor hub we integrated the light
sensor code implemented in lab 2 to make it work.
First we configured the UART by initializing ConfigureUART() and to display the floating point
values returned by the ISL29023DataLightVisibleGetFloat(&g_sISL29023Inst, &fAmbient)
where fAmbient contained the floating lux values.

And then defined the SYSTICKS_PER_SECOND and SYSTICK_PERIOD_MS, the
SysTickIntHandler(). Also configured and enabled SysTick Timer in main function.


Also defined ADC parameter to be true on PD0.

The program then reads the values, stores in the buffer and sends the data to the using sockets.

Then for the server to receive the sensor data, we configured the socket and buffer code for
receiving the data without any errors.

To plot real-time sensor reading we used the python PySerial API to display the serial data from
the board in floating point.


We used the matplotlib.pyplot python library. The readings were more sensitive when the
time.sleep() was reduced to 0.01s.

The challenges involved in the implementation were setting the right paths to several header
files. Also binding the client socket to server socket with the right IP address and port number.
Further there were difficulties in implementing and configuring the system to read the light
sensor data as there several errors to debug in the process like undefined symbols, missing
handlers in startup_rvmdk.s as well as ADC() function errors. The errors were overcome by
several hours of coding and debugging skills along with correct setting of the analog data reading
pin and clearing those which were overlapping with it.

IV. System verification and demonstration

For Task A, the system gave the correct temperature in fahrenheits and weather condition
information as per the openweathermap.org organization.

For Task B the system successfully established a client server communication along with real
time sensor transmission and real time plotting of the light sensor data.


   - Server ready to receive sensor data on IP address 192.168.43.251 and Port 5001 via COM
- Client sending light receive sensor data on IP address 192.168.43.251 and Port 5001 via COM


```
Real-time plot of the sensor data
```
V. Conclusion

Overall the lab was successfully demonstrated and helped to implement concepts like making
calls to the socket API, making a connection, transmit/receive application data and close the
connection as well as collecting sensor data with the combination of Tiva C Series
TM4C123GH6PM, CC3100 SimpleLink Wi-Fi Boostpack, Tiva Sensor Hub. The lab really
tested some of my coding and debugging skills which gives me confidence to further work in
respective industrial organizations.


