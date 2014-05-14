#ifndef _VISUALIZER_DRIVER_H
#define _VISUALIZER_DRIVER_H

#include <linux/ioctl.h>

#define VISUALIZER_MAGIC 4112

typedef struct {
    int16_t real;
    int16_t imag;
} complex_num;
typedef struct {
    int addr;
    int height; 
} freq_slot;

/* ioctls and their arguments */
#define VISUALIZER_WRITE_FREQ _IOW(VISUALIZER_MAGIC, 1, u32 *) //writes to freq_spec.sv 

#endif
