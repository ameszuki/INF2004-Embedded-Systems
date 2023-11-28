# The Idea
Develop Proof of Concept EDR(Endpoint Detection and Response) solution on Pico. 2 Picos will be involved, the 1st pico will be emulating a simple IoT device which will be monitored by the a 2nd pico. The 2nd pico will be the EDR that is monitoring the emulated IoT device's ram and flash memory for changes in configurations, voltage, etc.
A concrete example would be, detecting changes in the firmware of the emulated IoT device. The EDR will then send a reset signal as a response to prevent the anomaly from taking effect. Data of such anomaly will also be recorded and stored in an SD card. Data being monitored will also serve a web GUI for the viewing of data.

# Current use case
+ Target Pico - contains code that increments a counter variable to simulate the constant changing of value of an IoT device.

+ EDR Pico - proof of concept code utilising SWD protocol to read memory contents located in the target pico.

Please refer to diagrams below for visual aid.

# Pin Connections
```
+----------------+----------+
|  Target Pico   | EDR Pico |
+----------------+----------+
| swd gnd(Black) |        3 |
| swdio(Orange)  |        4 |
| swdclk(Yellow) |        5 |
| 2              |        6 |
| 1              |        7 |
+----------------+----------+
```

# Requirements
## Hardware
+ You will need
    - Raspberry Pi Pico x2
    - Maker Pi Pico x2
    - A bunch of jumper cables (Minimum 3)
    - SWD cable x1
    - USB to micro usb cable with data transfers x1
    - Micro SD Card x1

## Software
+ Firstly, you will have to install and set up the environment. Refer to below:
    - [Data Sheet](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf?_gl=1*dprr7b*_ga*MTU4NzkwNDY1OS4xNjkzODA1MDYw*_ga_22FD70LWDS*MTcwMTE5OTgxOC4xNS4xLjE3MDExOTk4MzMuMC4wLjA.)
    - [For C, Website](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html)
    - [For python, Website](https://www.raspberrypi.com/documentation/microcontrollers/micropython.html#what-is-micropython)


# Usage Sample Outputs
+ Available commands when the program is running
\
```
 _________________  ______ _____ _____ _____ 
|  ___|  _  \ ___ \ | ___ \_   _/  __ \  _  |
| |__ | | | | |_/ / | |_/ / | | | /  \/ | | |
|  __|| | | |    /  |  __/  | | | |   | | | |
| |___| |/ /| |\ \  | |    _| |_| \__/\ \_/ /
\____/|___/ \_| \_| \_|    \___/ \____/\___/ 
                                             
                                             
 Supported commands:

     "h" = Show this menu

     "s" = Perform SWD scan

     "w" = Save Dumped Data into SD Card

     "p" = Read Dumped Data from SD Card

     "d" = Toggle detection logic. Determines if detection logic will be performed in SWD Scan. Default false.

 [ Note: Disable 'local echo' in your terminal emulator program ]
```
\
+ SWD Scan
```
 > s

Enter number of dumps:
You entered: 4
Enter number of bytes you want to see per dump: 
You entered: 4
Enter number of bytes you want to see per line:
You entered: 4
Enter start address of dump: 
You entered: 0x20001364
Dump count set to: 4
Bytes per dump set to: 4
Total number of bytes: 16 
Number of bytes per line set to: 4
Start address set to: 0x20001364
Now starting dump operation...
===== DUMP: 3 ======
Command [0xa5]: ACK OK
     [  Pinout  ]  SWDIO=CH3 SWCLK=CH2

     [ Device 0 ]  0x0BC12477 (mfg: 'ARM Ltd' , part: 0xbc12, ver: 0x0)

= ABORT REGISTER =
Command [0x81]: ACK OK
= SELECT REGISTER =
Command [0xb1]: ACK OK
= TRANSFER ADDRESS REGISTER(TAR) ====
Command [0x8b]: ACK OK
= READ FROM DATA READ/WRITE(DRW) REGISTER =
Command [0x9f]: ACK OK
- Dumped bytes: 0x0
= READ FROM DATA READ/WRITE(DRW) REGISTER =
Command [0x9f]: ACK OK
- Dumped bytes: 0x63
storing data...
===== DUMP: 2 ======
Command [0xa5]: ACK OK
     [  Pinout  ]  SWDIO=CH3 SWCLK=CH2

     [ Device 0 ]  0x0BC12477 (mfg: 'ARM Ltd' , part: 0xbc12, ver: 0x0)

= ABORT REGISTER =
Command [0x81]: ACK OK
= SELECT REGISTER =
Command [0xb1]: ACK OK
= TRANSFER ADDRESS REGISTER(TAR) ====
Command [0x8b]: ACK OK
= READ FROM DATA READ/WRITE(DRW) REGISTER =
Command [0x9f]: ACK OK
- Dumped bytes: 0x0
= READ FROM DATA READ/WRITE(DRW) REGISTER =
Command [0x9f]: ACK OK
- Dumped bytes: 0x6c
storing data...
===== DUMP: 1 ======
Command [0xa5]: ACK OK
     [  Pinout  ]  SWDIO=CH3 SWCLK=CH2

     [ Device 0 ]  0x0BC12477 (mfg: 'ARM Ltd' , part: 0xbc12, ver: 0x0)

= ABORT REGISTER =
Command [0x81]: ACK OK
= SELECT REGISTER =
Command [0xb1]: ACK OK
= TRANSFER ADDRESS REGISTER(TAR) ====
Command [0x8b]: ACK OK
= READ FROM DATA READ/WRITE(DRW) REGISTER =
Command [0x9f]: ACK OK
- Dumped bytes: 0x0
= READ FROM DATA READ/WRITE(DRW) REGISTER =
Command [0x9f]: ACK OK
- Dumped bytes: 0x75
storing data...
===== DUMP: 0 ======
Command [0xa5]: ACK OK
     [  Pinout  ]  SWDIO=CH3 SWCLK=CH2

     [ Device 0 ]  0x0BC12477 (mfg: 'ARM Ltd' , part: 0xbc12, ver: 0x0)

= ABORT REGISTER =
Command [0x81]: ACK OK
= SELECT REGISTER =
Command [0xb1]: ACK OK
= TRANSFER ADDRESS REGISTER(TAR) ====
Command [0x8b]: ACK OK
= READ FROM DATA READ/WRITE(DRW) REGISTER =
Command [0x9f]: ACK OK
- Dumped bytes: 0x0
= READ FROM DATA READ/WRITE(DRW) REGISTER =
Command [0x9f]: ACK OK
- Dumped bytes: 0x81
storing data...
==== DUMPED DATA ====
0x0 0x0 0x0 0x63 
0x0 0x0 0x0 0x6c 
0x0 0x0 0x0 0x75 
0x0 0x0 0x0 0x81
```
\
+ Write to SD Card
```
> w

File opened.
File written.
File closed.
``` 
\
+ Read from SD Card
```
 > p

File opened.
File Read.
File closed.
```

# Dumping of memory to EDR pico for anomalies detection

To achieve a modular design process, our team came up with a flow chart and block diagram.
https://lucid.app/lucidchart/312a728e-e321-4c02-9378-942c1ac860c6/edit?viewport_loc=-2077%2C168%2C3543%2C1911%2C0_0&invitationId=inv_4bff7bb7-6d56-4b16-a35a-9eb6a69ea5cb

## Flow Chart:

![flowchart](https://github.com/CodeXTF2/INF2004-Embedded-Systems/blob/main/flowchart.JPG)

## Block Diagram:

![block diagram](https://github.com/CodeXTF2/INF2004-Embedded-Systems/blob/main/Block%20Diagram.JPG)

# References and Special Thanks
https://github.com/Aodrulez/blueTag
https://github.com/grandideastudio/jtagulator
https://research.kudelskisecurity.com/2019/05/16/swd-arms-alternative-to-jtag/
https://github.com/jbentham/picoreg
https://github.com/szymonh/SWDscan 
https://www.digikey.com/en/maker/projects/raspberry-pi-pico-rp2040-sd-card-example-with-micropython-and-cc/e472c7f578734bfd96d437e68e670050
https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico 