/* Creates frequency spectrum visualizer for different audio frequencies
 */ 

//#include "fbputchar.h"
#include <linux/stdio.h>
#include <linux/stdlib.h>
#include <linux/string.h>
#include <linux/sys/socket.h>
#include <linux/arpa/inet.h>
#include <linux/unistd.h>
//#include "usbkeyboard.h"
#include <linux/pthread.h>
#include <linux/math.h>
#include <linux/fs.h> //http://www.makelinux.net/ldd3/chp-3-sect-3
#include "visualizer_driver.h"

#define slot_values[] = {0, 31, 72, 150, 250, 440, 630, 1000, 2500, 5000, 8000, 14000, 20000};
#define slot_amps[12]; 
#define slot_heights[12]; 
#define SAMPLENUM 8192
#define SAMPLEBYTES SAMPLENUM*2
#define struct freq_bin freq_data[8192]; 

#define VISUALIZER_WRITE_FREQ _IOW(VISUALIZER_MAGIC, 1, u32 *) //writes to freq_spec.sv 
#define VISUALIZER_READ_FFT  _IOWR(VISUALIZER_MAGIC, 2, u32 *) //reads from aud_to_fft

void read_samples()
{
    int fd = fopen("aud_to_fft.vs", O_RDWR); //file descriptor? 
    struct freq_bin *freq_data;
    freq_data = (freq_bin*) malloc(SAMPLEBYTES); //allocating space for data array 
    
    if (ioctl(fd, FFT_DRIVER_READ_FFT, freq_data) == -1) //automatically updates freq_data? 
        printf("FFT_DRIVER_READ_FFT failed: %s\n",
            strerror(errno));
    else {
        if (status & FFT_DRIVER_READ_FFT)
            puts("FFT_DRIVER_READ_FFT is not set");
        else
            puts("FFT_DRIVER_READ_FFT is set");
    }
    
}

void write_samples(int* dataArray)
{
    int fd = open("freq_spec.vs", O_RDWR);    
    if (ioctl(fd, FFT_DRIVER_WRITE_FREQ, dataArray) == -1)
        printf("FFT_DRIVER_WRITE_FREQ failed: %s\n",
            strerror(errno));
    else {
        if (status & FFT_DRIVER_READ_FFT)
            puts("FFT_DRIVER_WRITE_FREQ is not set");
        else
            puts("FFT_DRIVER_WRITE_FREQ is set");
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
