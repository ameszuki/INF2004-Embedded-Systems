# The Idea
Develop Proof of Concept EDR(Endpoint Detection and Response) solution on Pico. 2 Picos will be involved, the 1st pico will be emulating a simple IoT device which will be monitored by the a 2nd pico. The 2nd pico will be the EDR that is monitoring the emulated IoT device's ram and flash memory for changes in configurations, voltage, etc.
A concrete example would be, detecting changes in the firmware of the emulated IoT device. The EDR will then send a reset signal as a response to prevent the anomaly from taking effect. Data of such anomaly will also be recorded and stored in an SD card. Data being monitored will also serve a web GUI for the viewing of data.

# Current use case
+ Target Pico - contains code that increments a counter variable to simulate the constant changing of value of an IoT device.

+ EDR Pico - proof of concept code utilising SWD protocol to read memory contents located in the target pico.

Please refer to diagrams below for visual aid.

# Pin Connections
+----------------+----------+\n
|  Target Pico   | EDR Pico |\n
+----------------+----------+\n
| swd gnd(Black) |        3 |\n
| swdio(Orange)  |        4 |\n
| swdclk(Yellow) |        5 |\n
| 2              |        6 |\n
| 1              |        7 |\n
+----------------+----------+\n

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