#ifndef _EQUALIZER_DRIVER_H
#define _EQUALIZER_DRIVER_H

#include <linux/ioctl.h>

struct send_info{
	uint8_t addr;
	uint8_t db; 
}; 

#define VGA_LED_MAGIC 'q'

/* ioctls and their arguments */
#define EQUALIZER_WRITE_DIGIT _IOW(EQUALIZER_MAGIC, 1, u8 *)

#endif
