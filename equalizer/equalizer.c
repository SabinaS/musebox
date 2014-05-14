//takes in 8k samples from fft_driver, equalizes them, passes them back to fft_driver

/*#include <linux/stdio.h>
#include <linux/unistd.h>
#include <linux/stdint.h>
#include <linux/sys/ioctl.h>
#include <linux/sys/types.h>
#include <linux/sys/stat.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/stdlib.h>
#include <linux/slab.h> */
#include "equalizer_driver.h"

#define EQUALIZER_WRITE_DIGIT _IOW(EQUALIZER_MAGIC, 1, u8 *)

#define DRIVER_NAME "equalizer_driver"
#define SAMPLENUM 8
#define SAMPLEBYTES SAMPLENUM*2

int fp; 

void write_db(u8* db_value, u8* addr)
{
    if (ioctl(addr, EQUALIZER_DRIVER_WRITE_DIGIT, db_value) == -1)
        printf("EQUALIZER_DRIVER_WRITE_DIGIT failed: %s\n",
            strerror(errno));
    else {
        if (status)
            puts("EQUALIZER_DRIVER_WRITE_DIGIT is not set");
        else
            puts("EQUALIZER_DRIVER_WRITE_DIGIT is set");
    }
}

int main()
{
    int freq,db;
    printf("Please type in the frequency you want to change (1 to 12):\n");
    scanf("%d",&freq);
    printf("Please type in the decibal value you would like to change it to (-12 to 12):\n");
    scanf("%d",&db);

    u8 addr, db_value; 
    addr = (u8 *) ("4'd" + freq - 1); 
    db_value = (u8 *) db; 

    write_db(db_value, addr); 
    
    printf("Equalizer Userspace program terminating\n");
    return 0;
}
