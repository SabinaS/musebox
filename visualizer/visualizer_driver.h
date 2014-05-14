#ifndef _VISUALIZER_DRIVER_H
#define _VISUALIZER_DRIVER_H

#include <linux/ioctl.h>

#define VGA_LED_MAGIC 'q'

struct freq_bin{
    int real;
    int imag;  
    double scaled_ampl; 
};

/* ioctls and their arguments */
#define VISUALIZER_WRITE_FREQ _IOW(VISUALIZER_MAGIC, 1, u32 *) //writes to freq_spec.sv 
#define VISUALIZER_READ_FFT  _IOWR(VISUALIZER_MAGIC, 2, u32 *) //reads from aud_to_fft

#endif
