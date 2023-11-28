/* 
    [ Read and write to SD Card on Raspberry Pi Pico Maker Board]
    [References & special thanks]
        https://www.digikey.com/en/maker/projects/raspberry-pi-pico-rp2040-sd-card-example-with-micropython-and-cc/e472c7f578734bfd96d437e68e670050
        https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico  
*/

#include "sd_card.h"
#include "ff.h"
#include "hw_config.h"

FRESULT f_result;
FATFS f_system;
FIL fp;
int ret;
char f_name[] = "dump";
UINT bytes_read;

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
    f_result = f_mount(&f_system, "0:", 1);
    if (f_result != FR_OK) 
    {
        printf("ERROR: Could not mount filesystem (%d)\r\n", f_result);
        while(true);
    }
    printf("Mount Success.\n");
}

void Write_File(char* buf, UINT numBytes)
{
    // Write something to file
    // this will overwrite anything currently in the sd card
    f_result = f_open(&fp, f_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (f_result != FR_OK) 
    {
        printf("ERROR: Could not open file (%d)\r\n", f_result);
        // while(true);
    }
    else
        printf("File opened.\n");
    f_result = f_write(&fp, buf, numBytes, &bytes_read);
    if (f_result != FR_OK) 
    {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        // while(true);
    }
    else
        printf("File written.\n");
    f_result = f_close(&fp);
    printf("File closed.\n");
}

void Read_File(char* buf, UINT numBytes, int format)
{
    f_result = f_open(&fp, f_name, FA_READ);
    if (f_result != FR_OK) 
    {
        printf("ERROR: Could not open file (%d)\r\n", f_result);
        // while(true);
    }
    else
        printf("File opened.\n");
    f_result = f_read(&fp, buf, numBytes, &bytes_read);
    if (f_result != FR_OK) 
    {
        printf("ERROR: Could not read file (%d)\r\n", ret);
        // while(true);
    }
    else
        printf("File Read.\n");
    f_result = f_close(&fp);

    printf("File closed.\n");

    printbuf(buf, numBytes, format);
}