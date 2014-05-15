/* 
 * Creates frequency spectrum visualizer for different audio frequencies
 */ 

//#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include "usbkeyboard.h"
#include <time.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CPU_AUDIO_US
#include "cpu_audio.h"

static struct timespec duration = {.tv_sec = 0, .tv_nsec = 23L};
static struct timespec remaining;

int main()
{
    char *file = "/dev/cpu_audio";
    int box_fd;

    struct sample *samples = (struct sample *) calloc(SAMPLENUM, sizeof(struct sample));
    int i;
    // 1 K sine wave
    // for (i = 0; i < SAMPLENUM; i++) {
    //     samples[i].left = 16383 * sin(250 * (2 * M_PI) * i / 44100);
    //     samples[i].right = 16383 * sin(250 * (2 * M_PI) * i / 44100);
    // }

    if ((box_fd = open(file, O_RDWR)) == -1 ) {
        fprintf(stderr, "could not open %s\n", file);
        return -1;
    }
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
    printf("size of sample: %u\n", sizeof(struct sample));
    //while (1) {
        while (ioctl(box_fd, CPU_AUDIO_READ_SAMPLES, samples)) {
            if (errno == EAGAIN) {
               //nanosleep(&duration, &remaining);
               continue;
            }
            fprintf(stderr, "errno: %d\n", errno);
            perror("ioctl read failed!");
            close(box_fd);
            return -1;
        }
        // for (i = 0; i < SAMPLENUM; i++)
        //     printf("%d: Sample left: %d, right: %d\n", i, samples[i].left, samples[i].right);
        printf("Sample location: %p\n", samples);
        if (ioctl(box_fd, CPU_AUDIO_WRITE_SAMPLES, samples)) {
            perror("ioctl write failed!");
            close(box_fd);
            return -1;
        }
    //}
    // Print out the values
    printf("left: %d, right %d\n", samples[0].left, samples[0].right);
    //     usleep(1/60.0 * 100000);
    //     if (height == 479)
    //         dir = 0;
    //     else if (height == 0)
    //         dir = 1;
    // }
    close(box_fd);
    return 0;
}
