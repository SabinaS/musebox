/*creates frequency spectrum visualizer for different audio frequencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "visualizer_driver.h"

#define USER_SPACE_FFT
#define VISUALIZER_MAGIC 4112
#define VISUALIZER_DRIVER_WRITE_FREQ _IOW(VISUALIZER_MAGIC, 1, int *)
#define VISUALIZER_DRIVER_READ_FFT _IOW(VISUALIZER_MAGIC, 1, int *)

#define SAMPLENUM 8192
#define H25K 4096

static int slot_values[12] = {31, 72, 150, 250, 440, 630, 1000, 2500, 5000, 8000, 14000, 20000};
static int bin_centers[12] = {6, 13, 28, 46, 82, 117, 186, 464, 929, 1486, 2601, 3715};
static int slot_heights[12];
static struct freq_slot slot_amps[12];
static struct complex_num freq_data[SAMPLENUM];

void read_samples()
{
    int fd = fopen("/dev/freq_spec", O_RDWR);
    struct complex_num *freq_data;
    freq_data = (complex_num*) malloc(SAMPLENUM*2);

    if (ioctl(fd, VISUALIZER_DRIVER_READ_FFT, freq_data) == -1)
        printf("VISUALIZER_DRIVER_READ_FFT failed: %s\n",
            strerror(errno));
    else {
        if (status & VISUALIZER_DRIVER_READ_FFT)
            puts("VISUALIZER_DRIVER_READ_FFT is not set");
        else
            puts("VISUALIZER_DRIVER_READ_FFT is set");
    }

}

void write_samples(int* dataArray)
{
    int fd = open("/dev/freq_spec", O_RDWR);
    if (ioctl(fd, VISUALIZER_DRIVER_WRITE_FREQ, dataArray) == -1){
        printf("VISUALIZER_DRIVER_WRITE_FREQ failed: %s\n", strerror(errno));
    }
    else {
        if (status){
            puts("VISUALIZER_DRIVER_WRITE_FREQ is not set");
        }
        else{
            puts("VISUALIZER_DRIVER_WRITE_FREQ is set");
        }
    }
}

int main()
{
    read_samples();
    int i, j;

    for(i= 1; i< 13; i++){
            double ampl_real = 0;
            double ampl_imag = 0;
                for(j=slot_values[i-1]; j< slot_values[i+1]; j++){
                    if(j<slot_values[i]){
                        ampl_real = ampl_real + (freq_data[i].real) * ((freq_data[j].real - slot_values[i-1])/(slot_values[i]- slot_values[i-1]));
                        ampl_imag = ampl_imag + (freq_data[i].imag) * ((freq_data[j].imag - slot_values[i-1])/(slot_values[i]- slot_values[i-1]));
                    }
                    else if(j>slot_values[i]){
                        ampl_real = ampl_real + (freq_data[i].real) * ((slot_values[i+1]-slot_values[i])/(slot_values[i+1]-freq_data[j].real));
                        ampl_imag = ampl_imag + (freq_data[i].imag) * ((slot_values[i+1]-slot_values[i])/(slot_values[i+1]-freq_data[j].imag));
                    }
                    else{
                        ampl_real = ampl_real + freq_data[j].real;
                        ampl_imag = ampl_imag + freq_data[j].imag;
                    }
                }

            double amp = (20*(log10((ampl_real)*(ampl_real) + (ampl_imag)*(ampl_imag))));
            slot_amps[i] = amp;
    }


    for( i = 1; i < 13; i++){
        int height;
        height = 479 - ((slot_amps[i])/186)*479;
        slot_heights[i] = height;

    }

    write_samples(slot_heights);

    printf("Visualizer Userspace program terminating\n");
    return 0;
}


    }

    write_samples(slot_heights);

    printf("Visualizer Userspace program terminating\n");
    return 0;
}

