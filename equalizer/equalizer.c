//takes in 8k samples from fft_driver, equalizes them, passes them back to fft_driver

#include <linux/stdio.h>
#include <linux/unistd.h>
#include <linux/stdint.h>
#include <linux/sys/ioctl.h>
#include <linux/sys/types.h>
#include <linux/sys/stat.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/stdlib.h>
#include <linux/slab.h> 
#include "equalizer_driver.h"

#define EQUALIZER_WRITE_DIGIT _IOW(EQUALIZER_MAGIC, 1, u32 *)

#define DRIVER_NAME "equalizer_driver"
#define SAMPLENUM 8
#define SAMPLEBYTES SAMPLENUM*2


void write_db(send_info* send)
{
    int fd = open("/dev/freq_spec", O_RDWR);    
    if (ioctl(fd, EQUALIZER_DRIVER_WRITE_DIGIT, send) == -1)
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
    printf("Please type in the frequency you want to change (0 to 11):\n");
    scanf("%d",&freq);
    printf("Please type in the decibal value you would like to change it to (1 to 25):\n");
    scanf("%d",&db);

    int addr, db_value; 
    addr = (int) freq; 
    db_value = (int) db; 
    
    struct send_info sendi;
    sendi = (send_info *) malloc(SAMPLEBYTES);
    sendi.addr = addr; 
    sendi.db = db_value; 

    write_db(sendi); 
    
    printf("Equalizer Userspace program terminating\n");
    return 0;
}
