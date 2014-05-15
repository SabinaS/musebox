#ifndef _VISUALIZER_DRIVER_H
#define _VISUALIZER_DRIVER_H

#include <linux/ioctl.h>

#define VISUALIZER_MAGIC 4112

typedef struct {
    int addr;
    int height; 
} freq_slot;

/* ioctls and their arguments */
#define VISUALIZER_WRITE_FREQ _IOW(VISUALIZER_MAGIC, 1, freq_slot *) //writes to freq_spec.sv 

#endif
