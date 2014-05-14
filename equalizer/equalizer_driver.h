#ifndef _EQUALIZER_DRIVER_H
#define _EQUALIZER_DRIVER_H

#include <linux/ioctl.h>

typedef struct{
	int addr;
	int db; 
}send_info; 

#define EQUALIZER_MAGIC 'q'

/* ioctls and their arguments */
#define EQUALIZER_DRIVER_WRITE_DIGIT _IOW(EQUALIZER_MAGIC, 1, u32 *)

#endif
