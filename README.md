# The Idea

The idea was to develop a Proof of Concept EDR (Endpoint Detection and Response) solution for IoT devices using a pico. The proof of concept involves 2 picos, where the first pico emulates a simple IoT device which is being monitored by the 2nd pico, which is the EDR. The goal is to Serial Wire Debug (SWD) interface to achieve a hardware level read/write of memory, which should be impossible for an attacker to tamper with from a software level. This can be used to monitor for anomalies in the flash memory such as configurations, voltage etc.

A concrete example would be, detecting changes in the firmware of the emulated IoT device. The EDR could then send a reset signal as a response to prevent the anomaly from taking effect. Data of such anomaly will also be recorded and stored in an SD card.  

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

To achieve a modular design process, our team came up with a flow chart and block diagram.
https://lucid.app/lucidchart/312a728e-e321-4c02-9378-942c1ac860c6/edit?viewport_loc=-2077%2C168%2C3543%2C1911%2C0_0&invitationId=inv_4bff7bb7-6d56-4b16-a35a-9eb6a69ea5cb

## SWD Introduction
+ Serial Wire Debug(SWD) interface was created as an alternative to JTAG. SWD solves the problem of JTAG whereby it uses only **2 signals (SWDCLK and SWDIO)** compared to JTAG's 4 signals (TCK, TMS, TDI, TDO). SWD allows debugging and accessing microprocessor registers, however its implementation is purely based on which processor you are using. For our project, **Raspberry Pi Pico uses ARM Cortex processors**. This will affect how one designs their code.
    - SWDCLK -- clock signal sent by host.
    - SWDIO -- bidirectional signal carrying data.
+ I will not be going in depth to describe the architecture, these information will be available under the reference section.

### SWD Transactions
![SWD Transaction](https://github.com/CodeXTF2/INF2004-Embedded-Systems/blob/main/img/SWDTransaction.JPG)
+ SWD Transactions are the basis of most of the operations. Aside from initialisation, reading and writing data to the target will take this form.
+ According to the specification, there are 3 phases to a SWD transaction.
    1. send a command
    2. receive an acknowledgement
    3. read/write data + parity
        - parity bit -- if there is odd number of 1s, parity = 1. It should be 0 otherwise.

+ Things to note:
    - when writing data to the target, you are writing data in Little Endian format.
    - when reading data from the target, your are receiving data in Big Endian format.

### Reading data for our use case
+ Reading data from the target requires a few steps which will be explained here.

+ Things to note:
    - There are 2 types of ports to take note here:
        - Debug Ports (DP)
        - Memory Access Ports (AP)
    - In short, the debug port handles how we access access ports that allows us to interact with the target's memory.
    - The APBANKSEL field in the SELECT REGISTER will affect how you access memory-ap registers
    - The TRANSFER ADDRESS REGISTER(TAR) holds the starting address of memory access. This affects subsequent read/write memory memory accesses.
    - The DATA READ WRITE(DRW) REGISTER, reads/writes data from the address held in TAR.

1. Firstly, we access the ABORT Register and clear and outstanding error flags.
    - command [ 0x81 ]
    - wdata [ 0x1e ]
2. Then, we fill the SELECT REGISTER.
    - command [ 0xb1 ]
    - wdata [ 0x0 ]
3. Proceed to access TAR
    - command [ 0x8b]
    - wdata [ INPUT ADDRESS HERE ]
4. Finally, read data from DRW register
    - command [ 0x9f ]
    - rdata read 32bits
    - rdata read parity 1bit
     


## Detection Logic

Our detection logic curently works as follows:

The first 10 (configurable in code) memory dumps are considered "known good" and are used as a baseline to detect expected behaviour patterns. The analysis currently is as follows:

### Static bytes
Bytes that are constantly static throughout the baseline data are marked as "static". Changes to any of these will be considered an anomaly and alerted on.
  
### Counters
Bytes that have a constant increment BETWEEN baseline dumps e.g. incrementing from 1 to 5 to 10 or 7 to 4 to 1 are considered "counters". If these bytes change by any offset other than the expected increment, it will be considered an anomaly and alerted on.
  
### Entropy
The overall entropy of each memory dump is calculated, and the average entropy of all baseline dumps is taken. If any future dumps have an entropy deviating from the average by more than 50% of the original average, it is considered an anomaly and alerted on.

These current detection rules are focused on detecting deviation from the "known good" rather than "known bad" e.g. static malware signatures in memory, to increase effectiveness against previously unknown attacks. A practical example of an attack being stopped by this would be buffer overflow exploits, where large chunks of memory may be overwritten suddenly. This would likely change bytes that were previously static, overwrite a few counter values unexpectedly and change the overall entropy of the memory by a significant amount due to the large amount of new data (such as "\x41\x41\x41\x41\x41") being placed into memory during the attack.

![image](https://github.com/CodeXTF2/INF2004-Embedded-Systems/assets/29991665/35daa0cd-fb50-460d-bea6-5ad90b48289d)



### Flow Chart:

![flowchart](https://github.com/CodeXTF2/INF2004-Embedded-Systems/blob/main/img/flowchart.JPG)

## Block Diagram:

![block diagram](https://github.com/CodeXTF2/INF2004-Embedded-Systems/blob/main/img/Block%20Diagram.JPG)
  
  
## Future work
This project is currently developed as a Proof of Concept (PoC) and has several limitations that should be addressed before it can be considered operationally practical to implement.  
- memory dumping speed is currently too slow to practically dump the entire flash memory (2MB) of a pi pico, which should be implemented for a real use case
- currently logs are stored in the SD card, displaying them via a web gui would be more user friendly  
- detection logic is currently hardcoded into the firmware. A more practical solution would be to send the raw dumps to a dedicated server/cloud in a secure manner, where more sophisticated analysis can be performed e.g.
    - YARA signature checking to detect known malware
    - forwarding to SIEM for query
 



Once these limitations are addressed, it would be implemented by attaching the pins that are currently connected to the slave pico via jumper cables to the IOT device's SWD interface, and having the EDR pico either airgapped or connected to a separate local network to prevent attackers from being able to reach the EDR pico.

# References and Special Thanks
https://github.com/Aodrulez/blueTag  
https://github.com/grandideastudio/jtagulator  
https://research.kudelskisecurity.com/2019/05/16/swd-arms-alternative-to-jtag/  
https://github.com/jbentham/picoreg  
https://github.com/szymonh/SWDscan   
https://www.digikey.com/en/maker/projects/raspberry-pi-pico-rp2040-sd-card-example-with-micropython-and-cc/e472c7f578734bfd96d437e68e670050  
https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico   
https://developer.arm.com/documentation/ihi0031/a/Overview-of-the-ARM-Debug-Interface-and-its-components
