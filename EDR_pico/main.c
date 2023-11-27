/* 
    [ blueTag - JTAGulator alternative based on Raspberry Pi Pico ]

        Inspired by JTAGulator. 

    [References & special thanks]
        https://github.com/grandideastudio/jtagulator
        https://research.kudelskisecurity.com/2019/05/16/swd-arms-alternative-to-jtag/
        https://github.com/jbentham/picoreg
        https://github.com/szymonh/SWDscan
        Arm Debug Interface Architecture Specification (debug_interface_v5_2_architecture_specification_IHI0031F.pdf)  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "detection.h"
#include "sd_card.h"
#include "ff.h"
#include "hw_config.h"


#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))
#define SIZE 1000
const uint onboardLED = 25;
char cmd;
char data_buf[SIZE];
char read_buf[SIZE];
bool isDetecting = false;


// include file from openocd/src/helper
static const char * const jep106[][126] = {
#include "jep106.inc"
};

FRESULT fr;
FATFS fs;
FIL fp;
int ret;
char filename[] = "dump";
UINT bytes_read;

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

//-------------------------------------SWD Scan [custom implementation]-----------------------------
     

#define LINE_RESET_CLK_CYCLES 52        // Atleast 50 cycles, selecting 52 
#define LINE_RESET_CLK_IDLE_CYCLES 2    // For Line Reset, have to send both of these
#define SWD_DELAY 5
#define JTAG_TO_SWD_CMD 0xE79E
#define SWDP_ACTIVATION_CODE 0x1A
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

uint xSwdClk=2;
uint xSwdIO=3;
bool swdDeviceFound=false;

// ========= DEFAULT VALUES FOR MEMORY READING =========
int numDumps = 1;
int bytes = 4;
int totalbytes = 4;
int format;
long addr;
int threshold = SIZE;
char buf1[SIZE];
int buf_counter = 0;

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
    printf("writing 0x%x\n", value);
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

// Receive ACK response from SWD device & verify if OK
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

// Leave dormant state
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

void dumpdata(char* buf, int* buf_counter)//long addr, int bytes
{
    long parity = getparity(addr);
    // 0x81 clear error flags
    printf("==== CLEAR ERROR FLAGS ====\n");
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
    printf("==== SELECT AP REGISTER ====\n");
    swdWriteBits(0xb1,8);
    swdTurnAround();
    if(swdReadAck(0xb1))
    {
        swdSetWriteMode();
        swdWriteBits(0x0,32); // select transfer address register
        swdWriteBits(0x0,1);
        swdWriteBits(0x00, 4);
    }

    // write operation (input start memory address here)
    printf("==== INPUT START MEMORY ADDRESS ====\n");
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
    // 1st 0x9f will always be 0x0, TLDR this is a dummy read
    for (int i = 0; i < bytes / 4 + 1; i++) {
        printf("==== READ FROM RDBUFF ====\n");
        swdWriteBits(0x9f, 8);
        swdTurnAround();
        if (swdReadAck(0x9f)) 
        {
            long data;
            data = swdRead(32);
            swdRead(1);
            printf("- Dumped bytes: 0x%x\n", data);
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

void swdTest()
{
    printf("=========TRYING TO READ FLASH==========\n");
    // 0x81 ABORT REGISTER
    printf("==== ABORT REGISTER, CLEAR ERROR FLAGS ====\n");
    // swdSetWriteMode();
    swdWriteBits(0x81,8);
    swdTurnAround();
    if(swdReadAck(0x81))
    {
        printf("0x81 ack received\n");
        // swdTurnAround();
        swdSetWriteMode();
        swdWriteBits(0x10,32);
        swdWriteBits(0x1,1);
        swdWriteBits(0x00, 4);
    }
    
    // // 0xb1 set APSEL and APBANKSEL
    // printf("==== set APSEL and APBANKSEL ====\n");
    // swdWriteBits(0xb1,8);
    // swdTurnAround();
    // if(swdReadAck(0xb1))
    // {
    //     printf("0xb1 ack received\n");
    //     // swdTurnAround();
    //     swdSetWriteMode();
    //     swdWriteBits(0x0,32);
    //     swdWriteBits(0x0,1);
    //     swdWriteBits(0x00, 4);
    // }

    // 0xa3 CSW set auto increment
    printf("==== CSW, SET AUTO INCREMENT ====\n");
    swdWriteBits(0xa3,8);
    swdTurnAround();
    if(swdReadAck(0xa3))
    {
        printf("0xa3 ack received\n");
        // swdTurnAround();
        swdSetWriteMode();
        swdWriteBits(0xa2000020,32);
        swdWriteBits(0x0,1);
        swdWriteBits(0x00, 4);
    }

    // // 0x87 CSW read
    // printf("==== CSW READ ====\n");
    // swdWriteBits(0x87,8);
    // swdTurnAround();
    // if(swdReadAck(0x87))
    // {
    //     printf("0x87 ack received\n");
    //     // swdTurnAround();
    //     swdRead(32);
    //     swdRead(1);
    // }
    // swdSetWriteMode();
    // swdWriteBits(0x00, 4);

    // // 0xb1 set APSEL and APBANKSEL
    // printf("==== set APSEL and APBANKSEL ====\n");
    // swdWriteBits(0xb1,8);
    // swdTurnAround();
    // if(swdReadAck(0xb1))
    // {
    //     printf("0xb1 ack received\n");
    //     // swdTurnAround();
    //     swdSetWriteMode();
    //     swdWriteBits(0x0,32);
    //     swdWriteBits(0x0,1);
    //     swdWriteBits(0x00, 4);
    // }
    
    // write operation (input start memory address here)
    printf("==== INPUT START MEMORY ADDRESS ====\n");
    swdWriteBits(0x8b,8);
    swdTurnAround();
    if(swdReadAck(0x8b))
    {
        printf("0x8b ack received\n");
        swdSetWriteMode();
        swdWriteBits(0x100001e8,32); // start of flash address
        swdWriteBits(0x0,1);
        swdWriteBits(0x00, 4);
    }

    printf("==== DUMMY READ ====\n");
    swdWriteBits(0x9f,8);
    swdTurnAround();
    if(swdReadAck(0x9f))
    {
        swdRead(32);
        swdRead(1);
    }
    swdSetWriteMode();
    swdWriteBits(0x00, 4);

    printf("==== READ DATA ====\n");
    swdWriteBits(0x9f,8);
    swdTurnAround();
    if(swdReadAck(0x9f))
    {
        swdRead(32);
        swdRead(1);
    }
    swdSetWriteMode();
    swdWriteBits(0x00, 4);

    printf("==== READ DATA ====\n");
    swdWriteBits(0x9f,8);
    swdTurnAround();
    if(swdReadAck(0x9f))
    {
        swdRead(32);
        swdRead(1);
    }
    swdSetWriteMode();
    swdWriteBits(0x00, 4);

    printf("==== READ DATA ====\n");
    swdWriteBits(0xbd,8);
    swdTurnAround();
    if(swdReadAck(0xbd))
    {
        swdRead(32);
        swdRead(1);
    }
    swdSetWriteMode();
    swdWriteBits(0x00, 4);
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

void printbuf(char* buf, int size, int format)
{
    printf("==== DUMPED DATA ====\n");
    for(int i = 0; i < size; i++)
    {
        if (i != 0 && i % format == 0)
            printf("\n");
        printf("0x%x ", buf[i]);
        
    }
    printf("\n\n");
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

    // COPY TO GLOBAL BUFFER FOR DETECTION LOGIC
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

void FileSystem_Init()
{
    // Initialize SD card
    if (!sd_init_driver()) 
    {
        printf("ERROR: Could not initialize SD card\r\n");
        while(true);
    }
    printf("SD Driver Init Success.\n");
}

void Mount_Drive()
{
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) 
    {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while(true);
    }
    printf("Mount Success.\n");
}

void Write_File()
{
    // Write something to file
    // Be aware that this will overwrite anything that is currently in the sd card
    fr = f_open(&fp, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) 
    {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        // while(true);
    }
    else
        printf("File opened.\n");
    fr = f_write(&fp, data_buf, totalbytes, &bytes_read);
    if (fr != FR_OK) 
    {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        // while(true);
    }
    else
        printf("File written.\n");
    fr = f_close(&fp);
    printf("File closed.\n");
}

void Read_File()
{
    // Write something to file
    // Be aware that this will overwrite anything that is currently in the sd card
    fr = f_open(&fp, filename, FA_READ);
    if (fr != FR_OK) 
    {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        // while(true);
    }
    else
        printf("File opened.\n");
    fr = f_read(&fp, read_buf, totalbytes, &bytes_read);
    if (fr != FR_OK) 
    {
        printf("ERROR: Could not read file (%d)\r\n", ret);
        // while(true);
    }
    else
        printf("File Read.\n");
    fr = f_close(&fp);
    printbuf(read_buf, totalbytes, format);
    printf("File closed.\n");
}

void init_buffers()
{
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        data_buf[i] = (char)0x0;
        read_buf[i] = (char)0x0;
    }
}

//--------------------------------------------Main--------------------------------------------------

int main()
{
    // sleep_ms(3000);
    stdio_init_all();
    init_buffers();
    FileSystem_Init();
    Mount_Drive();
    // GPIO init
    gpio_init(onboardLED);
    gpio_set_dir(onboardLED, GPIO_OUT);
    
    FileSystem_Init();
    Mount_Drive();

    //get user input to display splash & menu    
    cmd=getc(stdin);
    showMenu();
    showPrompt();

    while(1)
    {
        cmd=getc(stdin);
        printf("%c\n\n",cmd);
        switch(cmd)
        {
            // Help menu requested
            case 'h':
                showMenu();
                break;
            case 's':
                swdScan();
                break;
            case 'w':
                Write_File();
                break;
            case 'p':
                Read_File();
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
