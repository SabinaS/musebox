#ifndef _EQUALIZER_DRIVER_H
#define _EQUALIZER_DRIVER_H

#include <linux/ioctl.h>

typedef struct{
	uint8_t addr;
	uint8_t db; 
}send_info; 

#define VGA_LED_MAGIC 'q'

/* ioctls and their arguments */
#define EQUALIZER_DRIVER_WRITE_DIGIT _IOW(EQUALIZER_MAGIC, 1, u8 *)

#endif
