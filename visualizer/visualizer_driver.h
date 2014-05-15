#ifndef _VISUALIZER_DRIVER_H
#define _VISUALIZER_DRIVER_H

#include <linux/ioctl.h>
#include "visualizer.h"

#define VGA_LED_MAGIC 'q'

<<<<<<< HEAD
struct freq_bin{
    int real;
    int imag;  
    double scaled_ampl; 
};

/* ioctls and their arguments */
#define VISUALIZER_WRITE_FREQ _IOW(VISUALIZER_MAGIC, 1, u32 *) //writes to freq_spec.sv 
#define VISUALIZER_READ_FFT  _IOWR(VISUALIZER_MAGIC, 2, u32 *) //reads from aud_to_fft
=======
typedef struct {
    int16_t real;
    int16_t imag;
} complex_num;
typedef struct {
    int addr;
    int height;
} freq_slot;

/* ioctls and their arguments */
#define VISUALIZER_DRIVER_WRITE_FREQ _IOW(VISUALIZER_MAGIC, 1, u32 *)  
#define VISUALIZER_DRIVER_READ_FFT _IOW(VISUALIZER_MAGIC, 1, u32 *) 
>>>>>>> 8f17aea50365c00a9a5cd9c00efc532a3ad25e8c

#endif

~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
~                                                                                                                       
"visualizer_driver.h" 22L, 429C                                                                       18,1          All
