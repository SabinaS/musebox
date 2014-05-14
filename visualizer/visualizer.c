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

#define USER_SPACE_FFT
typedef struct {
    int16_t real;
    int16_t imag;
} complex_num;
typedef struct {
    int addr;
    int height; 
} freq_slot;

#define SAMPLENUM 8192
#define H25K 4096
static int slot_values[12] = {31, 72, 150, 250, 440, 630, 1000, 2500, 5000, 8000, 14000, 20000};
static int bin_centers[12] = {6, 13, 28, 46, 82, 117, 186, 464, 929, 1486, 2601, 3715};
static struct complex_num freq_data[SAMPLENUM];

void read_samples()
{
    int fd = fopen("/dev/freq_spec", O_RDWR); //file descriptor? 
    struct freq_bin *freq_data;
    freq_data = (freq_bin*) malloc(SAMPLEBYTES); //allocating space for data array 
    
    if (ioctl(fd, VISUALIZER_DRIVER_READ_FFT, freq_data) == -1) //automatically updates freq_data? 
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
    if (ioctl(fd, VISUALIZER_DRIVER_WRITE_FREQ, dataArray) == -1)
        printf("VISUALIZER_DRIVER_WRITE_FREQ failed: %s\n",
            strerror(errno));
    else {
        if (status & VISUALIZER_DRIVER_READ_FFT)
            puts("VISUALIZER_DRIVER_WRITE_FREQ is not set");
        else
            puts("VISUALIZER_DRIVER_WRITE_FREQ is set");
    }
}

//read in from visualizer_driver audio frequncies 
//register changes in frequencies
//send changes to writedata to visualizer module (freq_spec.sv)

int main()
{
    read_samples(); //update freq_data
    int i = 0; 
    int j = 0; 
    
    //add the frequencies of each bin 
    //get the amplitude by 20log_10(reals^2 + imags^2)
    while(i= 1; i< 13; i++){
	    double ampl_real = 0; 
	    double ampl_imag = 0; 
		while(j=slot_values[i-1]; j< slot_values[i+1]; j++){
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
    
    //place heights between 0 and 479
    while(i=1; i < 13; i++){
	int height;
	height = 479 - ((slot_amps[i])/186)*479; 
	slot_heights[i] = height; 

    }
    
    write_samples(slot_heights); //writing the 8k samples 
    
    printf("Visualizer Userspace program terminating\n");
    return 0;
}
