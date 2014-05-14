//takes in 8k samples from fft_driver, equalizes them, passes them back to fft_driver

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <linux/slab.h>
#include "fft_driver.h"

#define FFT_DRIVER_WRITE_DIGIT _IOW(FFT_DRIVER_MAGIC, 1, u16 *)
#define FFT_DRIVER_READ_DIGIT  _IOWR(FFT_DRIVER_MAGIC, 2, u16 *)

#define DRIVER_NAME "fft_driver"
#define SAMPLENUM 8192
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
    scanf("%d",&dv);

    u8 addr, db_value; 
    addr = (u8 *) ("4'd" + freq - 1); 
    db_value = (u8 *) db; 

    write_db(db_value, addr); 
    
    printf("Equalizer Userspace program terminating\n");
    return 0;
}
