/* 
    [ swd_edr - Proof of Concept for Raspberry Pi Pico to be used as an Endpoint Detection and Response ]
    [References & special thanks]
        https://github.com/Aodrulez/blueTag
        https://github.com/grandideastudio/jtagulator
        https://research.kudelskisecurity.com/2019/05/16/swd-arms-alternative-to-jtag/
        https://github.com/jbentham/picoreg
        https://github.com/szymonh/SWDscan
        Arm Debug Interface Architecture Specification (debug_interface_v5_2_architecture_specification_IHI0031F.pdf)  
        https://www.digikey.com/en/maker/projects/raspberry-pi-pico-rp2040-sd-card-example-with-micropython-and-cc/e472c7f578734bfd96d437e68e670050
        https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "detection.h"
#include "filesystem.h"

#define LINE_RESET_CLK_CYCLES 52        // Atleast 50 cycles, selecting 52 
#define LINE_RESET_CLK_IDLE_CYCLES 2    // For Line Reset, have to send both of these
#define SWD_DELAY 5
#define JTAG_TO_SWD_CMD 0xE79E
#define SWDP_ACTIVATION_CODE 0x1A
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#define SIZE 1000

uint xSwdClk=2; // CLOCK PIN
uint xSwdIO=3; // IO PIN
bool swdDeviceFound=false;
int numDumps = 1; // number of dumps
int bytes = 4; // number of bytes per dump
int totalbytes = 4; // total bytes stored
int format; // number of bytes per line when printing
long addr = 0x100001e8; // start address of dump
int threshold = SIZE; // max number of bytes buffer can hold
const uint onboardLED = 25;
char cmd;
char data_buf[SIZE];
char read_buf[SIZE];
bool isDetecting = false;

static const char * const jep106[][126] = {
#include "jep106.inc"
};

void showPrompt(void)
{
    printf(" > ");
}

void showMenu(void)
{
    printf(" Supported commands:\n\n");
    printf("     \"h\" = Show this menu\n\n");
    printf("     \"s\" = Perform SWD scan\n\n");
    printf("     \"w\" = Save Dumped Data into SD Card\n\n");
    printf("     \"p\" = Read Dumped Data from SD Card\n\n");
    printf("     \"d\" = Toggle detection logic. Determines if detection logic will be performed in SWD Scan. Default false.\n\n");
    printf(" [ Note: Disable 'local echo' in your terminal emulator program ]\n\n");
}

const char *jep106_table_manufacturer(unsigned int bank, unsigned int id)
{
	if (id < 1 || id > 126) {
		return "Unknown";
	}
	/* index is zero based */
	id--;
	if (bank >= ARRAY_SIZE(jep106) || jep106[bank][id] == 0)
		return "Unknown";
	return jep106[bank][id];
}

void swdDisplayDeviceDetails(uint32_t idcode)
{
        printf("     [ Device 0 ]  0x%08X ",  idcode);
        uint32_t idc = idcode;
        long part = (idc & 0xffff000) >> 12;
        int bank=(idc & 0xf00) >> 8;
        int id=(idc & 0xfe) >> 1;
        int ver=(idc & 0xf0000000) >> 28;

        if (id > 1 && id <= 126 && bank <= 8) 
        {
            printf("(mfg: '%s' , part: 0x%x, ver: 0x%x)\n",jep106_table_manufacturer(bank,id), part, ver);
        }
    printf("\n");
}

void swdDisplayPinout(int swdio, int swclk, uint32_t idcode)
{
    printf("     [  Pinout  ]  SWDIO=CH%d", swdio);
    printf(" SWCLK=CH%d\n\n", swclk);
    swdDisplayDeviceDetails(idcode);
}

void initSwdPins(void)
{
    gpio_init(xSwdClk);
    gpio_init(xSwdIO);
    gpio_set_dir(xSwdClk,GPIO_OUT);
    gpio_set_dir(xSwdIO,GPIO_OUT);
}

void swdClockPulse(void)
{
    gpio_put(xSwdClk, 0);
    sleep_ms(SWD_DELAY);
    gpio_put(xSwdClk, 1);
    sleep_ms(SWD_DELAY);
}

void swdSetReadMode(void)
{
    gpio_set_dir(xSwdIO,GPIO_IN);
}

void swdTurnAround(void)
{
    swdSetReadMode();
    swdClockPulse();
}

void swdSetWriteMode(void)
{
    gpio_set_dir(xSwdIO,GPIO_OUT);
    swdClockPulse();
}

void swdIOHigh(void)
{
    gpio_put(xSwdIO, 1);
}

void swdIOLow(void)
{
    gpio_put(xSwdIO, 0);
}

void swdWriteHigh(void)
{
    gpio_put(xSwdIO, 1);
    swdClockPulse();
}

void swdWriteLow(void)
{
    gpio_put(xSwdIO, 0);
    swdClockPulse();
}

bool swdReadBit(void)
{
    bool value=gpio_get(xSwdIO);
    swdClockPulse();
    return(value);
}

void swdReadDPIDR(void)
{
    long buffer;
    bool value;
    for(int x=0; x< 32; x++)
    {
        value=swdReadBit();
        bitWrite(buffer, x, value);
    }
    swdDisplayPinout(xSwdIO, xSwdClk, buffer);
}

long swdRead(int bit_count)
{
    long buffer;
    bool value;
    for(int x=0; x < bit_count; x++)
    {
        value=swdReadBit();
        bitWrite(buffer, x, value);
    }
    // printf("Reading 0x%x\n", buffer);
    
    return buffer;
}

void swdWriteBit(bool value)
{
    gpio_put(xSwdIO, value);
    swdClockPulse();
}

void swdWriteBits(long value, int length)
{
    // printf("writing 0x%x\n", value);
    for (int i=0; i<length; i++)
    {
        swdWriteBit(bitRead(value, i));
    }
}

bool clearAndCheck(long command)
{
    
    // 0x81 clear error flags
    swdSetWriteMode();
    swdWriteBits(0x81,8);
    swdTurnAround();
    bool bit1=swdReadBit();
    bool bit2=swdReadBit();
    bool bit3=swdReadBit();
    if(bit1 == true && bit2 == false && bit3 == false)
    {
        printf("Clearing error flags...\n");
        swdSetWriteMode();
        swdWriteBits(0x1e,32);
        swdWriteBits(0x0,1);
        swdWriteBits(0x00, 4);
        return true;
    }
    return false;
}

bool swdReadAck(long command)
{
    bool bit1=swdReadBit();
    bool bit2=swdReadBit();
    bool bit3=swdReadBit();
    long prev_comand = command;
    // printf("%d%d%d\n", bit1, bit2, bit3);
    if(bit1 == true && bit2 == false && bit3 == false)
    {
        printf("Command [0x%x]: ACK OK\n", prev_comand);
        return true;
    }
    else
    {
        printf("Command [0x%x]: ACK NOT OK, %d%d%d\n", prev_comand, bit1, bit2, bit3);
        printf("Clearing error flags and retrying...\n");
        return clearAndCheck(prev_comand);
    }
}

void swdResetLineSWDJ(void)
{
    swdIOHigh();
    for(int x=0; x < LINE_RESET_CLK_CYCLES+10; x++)
    {
        swdClockPulse();
    }
}

void swdResetLineSWD(void)
{
    swdIOHigh();
    for(int x=0; x < LINE_RESET_CLK_CYCLES+10; x++)
    {
        swdClockPulse();
    }
    swdIOLow();
    swdClockPulse();
    swdClockPulse();
    swdClockPulse();
    swdClockPulse();
    swdIOHigh();
}

void swdArmWakeUp(void)
{
    swdSetWriteMode();
    swdIOHigh();
    for(int x=0;x < 8; x++)     // Reset to selection Alert Sequence
    {
        swdClockPulse();
    }

    // Send selection alert sequence 0x19BC0EA2 E3DDAFE9 86852D95 6209F392 (128 bits)
    swdWriteBits(0x92, 8);
    swdWriteBits(0xf3, 8);
    swdWriteBits(0x09, 8);
    swdWriteBits(0x62, 8);

    swdWriteBits(0x95, 8);
    swdWriteBits(0x2D, 8);
    swdWriteBits(0x85, 8);
    swdWriteBits(0x86, 8);

    swdWriteBits(0xE9, 8);
    swdWriteBits(0xAF, 8);
    swdWriteBits(0xDD, 8);
    swdWriteBits(0xE3, 8);

    swdWriteBits(0xA2, 8);
    swdWriteBits(0x0E, 8);
    swdWriteBits(0xBC, 8);
    swdWriteBits(0x19, 8);

    swdWriteBits(0x00, 4);   // idle bits
    swdWriteBits(SWDP_ACTIVATION_CODE, 8);
}

void swdinitialop()
{
    swdSetWriteMode();
    swdArmWakeUp();                     // Needed for devices like RPi Pico
    swdResetLineSWDJ();
    swdWriteBits(JTAG_TO_SWD_CMD, 16);
    swdResetLineSWDJ();
    swdWriteBits(0x00, 4);
    swdWriteBits(0xA5, 8);             // readIdCode command 0b10100101
    swdTurnAround();                   // synch clk and set to read mode
    if(swdReadAck(0xA5) == true)           // Got ACK OK
    {
        swdDeviceFound=true;
        swdReadDPIDR();
    }
    swdSetWriteMode();
    swdWriteBits(0x00, 4);
}

long getparity(long data)
{
    int counter = 0;
    for(int i = 0; i < 32; i++)
    {
        if((data >> i) & 1)
        {
            ++counter;
        }
    }
    if(counter % 2)
        return 0x1;
    
    return 0x0;
}

void dumpdata(char* buf, int* buf_counter)
{
    long parity = getparity(addr);
    // 0x81 clear error flags
    printf("= ABORT REGISTER =\n");
    swdWriteBits(0x81,8);
    swdTurnAround();
    if(swdReadAck(0x81))
    {
        swdSetWriteMode();
        swdWriteBits(0x1e,32);
        swdWriteBits(0x0,1);
        swdWriteBits(0x00, 4);
    }

    // 0xb1 select AP register
    printf("= SELECT REGISTER =\n");
    swdWriteBits(0xb1,8);
    swdTurnAround();
    if(swdReadAck(0xb1))
    {
        swdSetWriteMode();
        swdWriteBits(0x0,32); // select transfer address register(TAR)
        swdWriteBits(0x0,1);
        swdWriteBits(0x00, 4);
    }

    // write operation (input start memory address here)
    printf("= TRANSFER ADDRESS REGISTER(TAR) ====\n");
    swdWriteBits(0x8b,8);
    swdTurnAround();
    if(swdReadAck(0x8b))
    {
        swdSetWriteMode();
        swdWriteBits(addr,32); // start of address to dump,
        swdWriteBits(parity,1);
        swdWriteBits(0x00, 4);
    }

    // read from RDBUFF register
    // 1st 0x9f will always be 0x0, considered a dummy read
    for (int i = 0; i < bytes / 4 + 1; i++) {
        printf("= READ FROM DATA READ/WRITE(DRW) REGISTER =\n");
        swdWriteBits(0x9f, 8);
        swdTurnAround();
        if (swdReadAck(0x9f)) 
        {
            long data;
            data = swdRead(32);
            swdRead(1);
            printf("- Dumped bytes: 0x%x\n", data);
            // Skip dummy read
            if(i == 0)
            {
                swdSetWriteMode();
                swdWriteBits(0x00, 4);
                continue;
            }
                
            // Append data to buffer
            printf("storing data...\n");
            buf[(*buf_counter)++] = (char)((data >> 24) & 0xFF);
            // printf("stored 0x%x\n", (char)(data >> 24 & 0xFF));
            // printf("buf_counter: %d\n", *buf_counter);

            buf[(*buf_counter)++] = (char)((data >> 16) & 0xFF);
            // printf("stored 0x%x\n", (char)(data >> 16 & 0xFF));
            // printf("buf_counter: %d\n", *buf_counter);

            buf[(*buf_counter)++] = (char)((data >> 8) & 0xFF);
            // printf("stored 0x%x\n", (char)(data >> 8 & 0xFF));
            // printf("buf_counter: %d\n", *buf_counter);

            buf[(*buf_counter)++] = (char)(data & 0xFF);
            // printf("stored 0x%x\n", (char)(data & 0xFF));  
            // printf("buf_counter: %d\n", *buf_counter); 
        }
        swdSetWriteMode();
        swdWriteBits(0x00, 4);
    }
}

int getNumDumps()
{
    int dumps;

    printf("Enter number of dumps:\n");
    scanf("%d", &dumps);

    return dumps;
}

int getFormat()
{
    int format;

    printf("Enter number of bytes you want to see per line:\n");
    scanf("%d", &format);

    return format;
}

long getAddress()
{
    long addr;

    printf("Enter start address of dump: \n");
    scanf("%lx", &addr);

    return addr;
}

int getBytes()
{
    int _bytes;

    printf("Enter number of bytes you want to see per dump: \n");
    scanf("%d", &_bytes);

    return _bytes;
}

void swdTrySWDJ(void)
{
    int buf_counter = 0;
    char buf1[SIZE];
    numDumps = getNumDumps();
    bytes = getBytes();
    totalbytes = numDumps * bytes;
    format = getFormat();
    addr = getAddress();
    
    printf("Dump count set to: %d\n", numDumps);
    printf("Bytes per dump set to: %d\n", bytes);
    printf("Total number of bytes: %d \n", totalbytes);
    if(totalbytes >= threshold)
    {
        printf("Operation not permitted. Current maximum number of bytes is %d.\n Exiting program...\n", threshold);
        return 0;
    }
    printf("Number of bytes per line set to: %d\n", format);
    printf("Start address set to: 0x%x\n", addr);
    printf("Now starting dump operation...\n");

    while(numDumps--)
    {
        printf("===== DUMP: %d ======\n", numDumps);
        swdinitialop();
        dumpdata(buf1, &buf_counter);
        sleep_ms(1000);
    }

    // COPY TO GLOBAL BUFFER
    memcpy(data_buf, buf1, SIZE);

    printbuf(buf1, totalbytes, format);

    printf("\n");
    
}

bool swdBruteForce(void)
{
    // onBoard LED notification
    gpio_put(onboardLED, 1);
    swdTrySWDJ();
    gpio_put(onboardLED, 0);
    if(swdDeviceFound)
    { return(true); } else { return(false); }
}

void swdScan(void)
{ 
    
    swdDeviceFound = false;
    bool result = false;

    initSwdPins();
    result = swdBruteForce();
    // printf("xSwdIO PIN: %d\n", xSwdIO);
    // printf("xSwdClk PIN: %d\n", xSwdClk);
    // Process the remaining test cases
    if(swdDeviceFound == false)
    {
        printf("     No devices found. Please try again.\n\n");
        return 0;
    }

    //DETECTION LOGIC
    if(isDetecting)
    {
        for (int i = 0; i < totalbytes; i++) 
        {
            int result = detectMemoryAnomalies((unsigned char *)data_buf[i]);
            if (result) 
            {
                printf("Anomaly detected in test case %d\n", i + 1);
            } 
            else 
            {
                printf("No anomaly detected in test case %d\n", i + 1);
            }
        }
    }
    
}

void ToggleDetection()
{
    if(isDetecting)
    {
        isDetecting = false;
        printf("Detection turned off. %d\n", isDetecting);
        return;
    }
    if(!isDetecting)
    {
        isDetecting = true;
        printf("Detection turned on. %d\n", isDetecting);
        return;
    }
}

//--------------------------------------------Main--------------------------------------------------

int main()
{
    stdio_init_all();
    FileSystem_Init();
    Mount_Drive();
    // GPIO init
    gpio_init(onboardLED);
    gpio_set_dir(onboardLED, GPIO_OUT);

    //get user input to display splash & menu    
    showMenu();
    showPrompt();

    while(1)
    {
        // RECEIVE COMMANDS FROM STDIN
        cmd=getc(stdin);
        printf("%c\n\n",cmd);
        switch(cmd)
        {
            case 'h':
                showMenu();
                break;
            case 's':
                swdScan();
                break;
            case 'w':
                Write_File(data_buf, totalbytes);
                break;
            case 'p':
                Read_File(read_buf, totalbytes, format);
                
                break;
            case 'd':
                ToggleDetection();
                break;
            default:
                printf(" Unknown command. \n\n");
                break;
        }
        showPrompt();
    }    
    return 0;
}
