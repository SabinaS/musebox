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
#include "visualizer_driver.h"

#define USER_SPACE_FFT
struct complex_num {
    int16_t real;
    int16_t imag;
};

#define SAMPLENUM 8192
#define H25K 4096
static int slot_values[12] = {31, 72, 150, 250, 440, 630, 1000, 2500, 5000, 8000, 14000, 20000};
static int bin_centers[12] = {6, 13, 28, 46, 82, 117, 186, 464, 929, 1486, 2601, 3715};
static struct complex_num freq_data[SAMPLENUM];

int main()
{
    freq_slot slot;
    char *file = "/dev/visualizer";
    int frec_spec_fd;

    if ((frec_spec_fd = open(file, O_RDWR)) == -1 ) {
        fprintf(stderr, "could not open %s\n", file);
        return -1;
    }

    int bar = 0;
    int height = 240;
    int dir = 0;
    while (bar < 12) {
        if (dir == 0)
            height--;
        else
            height++;
        slot.addr = bar++;
        if (bar == 12)
            bar = 0;
        slot.height = height;
        if (ioctl(frec_spec_fd, VISUALIZER_WRITE_FREQ, &slot)) {
            perror("ioctl write failed!");
            close(frec_spec_fd);
            return -1;
        }
        usleep(1/60.0 * 100000);
        if (height == 479)
            dir = 0;
        else if (height == 0)
            dir = 1;
    }
    close(frec_spec_fd);
    return 0;
}
