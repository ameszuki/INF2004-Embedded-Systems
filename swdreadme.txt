Select Debug Port or Access Port (AP):

This step involves identifying and selecting the correct Debug Port (DP) or Access Port (AP) for the target device. The AP is responsible for managing access to various components on the device, such as memory or registers.
Send Command Code:

The command code sent after selecting the port often includes information about the register or operation you want to perform. For example, you might specify whether you are reading or writing, the address or register to access, and any additional parameters.
Receive Acknowledgement:

The acknowledgment not only confirms that the command was received but may also contain status information, error flags, or other details about the success or failure of the operation.
Proceed to Receive/Transmit Data:

When dealing with data transfer, such as reading or writing memory, you'll need to consider the size and format of the data (e.g., 8-bit, 16-bit, or 32-bit). The command may include information about the data transfer, such as the size and address.
Reset Protocol:

In some cases, a reset or synchronization step may be necessary to prepare for subsequent operations. This ensures that both the debugger and the target device are in a known state before the next interaction.
Additional Considerations:

SWD often involves a sequence of interactions to set up the debugging environment, halt the processor, and perform specific operations. For example, you might need to handle breakpoints, step through code, or examine processor status.

0x8E - 1000 1110
- DPACC
- write access
- 1000 = 0x8 + write = AP Select

0xA9 - 1010 1001
- DPACC
- read access
- 1000 = 0x8 + read = RESEND?

0xC5 - 1100 0101
- APACC
- Write
- 0000 = 0x0 + Write = 



C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\openocd>openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -s scripts -c "adapter speed 5000"

C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\gcc-arm-none-eabi\bin>arm-none-eabi-gdb D:\SIT\2004_lab\project\hello_usb.elf

D:\\SIT\\INF2004-EmbeddedSystemsProgramming\\project\\testdump

openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -s scripts -c "adapter speed 5000" -c "init" -c "reset init" -c "dump_image D:\\SIT\\INF2004-EmbeddedSystemsProgramming\\project\\testdump 0x100001e8 12" -c "exit"
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -s scripts -c "adapter speed 5000" -c "echo init" -c "init" -c "echo reset init" -c "reset init" -c "echo dumping" -c "dump_image D:\\SIT\\INF2004-EmbeddedSystemsProgramming\\project\\testdump 0x100001e8 12" -c "echo exiting" -c "exit"
arm-none-eabi-gdb D:\SIT\INF2004-EmbeddedSystemsProgramming\project\hell0_usb.elf


0xA5 -- READ IDCODE
0x81
0xb1
0x8d
0x8b -- put in address here
0x9f -- read bytes
0xbd -- read last 4 bytes
0xbd -- read 0
======= end operation ==========
SWD_transfer
Write 8 bits 0x8b
Read 4 bits 0x30000000 (shifted 0x3)
Write 32 bits 0xe000edf0
Write 1 bits 0x1
write 8b ack 01 0xe000edf0 parity d
SWD_transfer
Write 8 bits 0xb1
Read 4 bits 0x30000000 (shifted 0x3)
Write 32 bits 0x13
Write 1 bits 0x1
write b1 ack 01 0x00000013 parity 3
SWD_transfer
Write 8 bits 0x87
Read 4 bits 0x30000000 (shifted 0x3)
Read 32 bits 0x0 (shifted 0x0)
Read 1 bits 0x0 (shifted 0x0)
Read 87 ack 01 0x00000000 parity 0
SWD_transfer
Write 8 bits 0xbd
Read 4 bits 0x30000000 (shifted 0x3)
Read 32 bits 0x30003 (shifted 0x30003)
Read 1 bits 0x0 (shifted 0x0)
Read bd ack 01 0x00030003 parity 0
SWD_transfer
Write 8 bits 0xbd
Read 4 bits 0x30000000 (shifted 0x3)
Read 32 bits 0x0 (shifted 0x0)
Read 1 bits 0x0 (shifted 0x0)
Read bd ack 01 0x00000000 parity 0
Set swclk freq 1000KHz sysclk 125000kHz
Set swclk freq 4807KHz sysclk 125000kHz
SWJ sequence count = 64 FDB=0xff
Write 8 bits 0xff
Write 8 bits 0xff
Write 8 bits 0xff
Write 8 bits 0xff
Write 8 bits 0xff
Write 8 bits 0xff
Write 8 bits 0xff
Write 8 bits 0x0

select register = 0xb1 
write 0x0 32bits (APSEL = 0, APBANKSEL = 0)
parity 0

select CSW write = 0xA3 
write 0x23000012 32bits (0010 0011 0000 0000 0000 0000 0001 0010) (auto increment single)
parity 1
write 0x23000052 32bits (0010 0011 0000 0000 0000 0000 0101 0010) (device enable) (auto increment single)
parity 0
write 0xa2000012 (1010 0010 0000 0000 0000 0000 0001 0010)
parity 1
write 0xa3000012 (1010 0011 0000 0000 0000 0000 0001 0010)
parity 0

A2000022 (1010 0010 0000 0000 0000 0000 0010 0010)
parity 1

CSW read = 0x87
read 32bits

input TAR = 0x8b
write 0x100001e8 32bits
parity 0

DRW READ = 0x9f
read 32bits

    

0000 0000 1111 1111 1111 1111 0000 0000 - 0xFFFF00

counter variable
0x20001364

0x0 0x0 0x0 0x8 
0x0 0x0 0x0 0xf 
0x0 0x0 0x0 0x16 
0x0 0x0 0x0 0x1d 
0x0 0x0 0x0 0x24

