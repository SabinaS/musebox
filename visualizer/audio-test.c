/* 
 * Creates frequency spectrum visualizer for different audio frequencies
 */ 

//#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include "usbkeyboard.h"
#include <math.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CPU_AUDIO_US
#include "cpu_audio.h"

#define SAMPLENUM 8192
#define H25K 4096

int main()
{
    char *file = "/dev/cpu_audio";
    int box_fd;

    if ((box_fd = open(file, O_RDWR)) == -1 ) {
        fprintf(stderr, "could not open %s\n", file);
        return -1;
    }

    sample_t samples[SAMPLENUM];
    // int bar = 0;
    // int height = 240;
    // int dir = 0;
    // while (bar < 12) {
    //     if (dir == 0)
    //         height--;
    //     else
    //         height++;
    //     slot.addr = bar++;
    //     if (bar == 12)
    //         bar = 0;
    //     slot.height = height;
    if (ioctl(box_fd, CPU_AUDIO_READ_SAMPLES, samples)) {
        perror("ioctl write failed!");
        close(box_fd);
        return -1;
    }
    //     usleep(1/60.0 * 100000);
    //     if (height == 479)
    //         dir = 0;
    //     else if (height == 0)
    //         dir = 1;
    // }
    close(box_fd);
    return 0;
}
