#ifndef _EQUALIZER_H
#define _EQUALIZER_H

#include <linux/ioctl.h>

#define VGA_LED_MAGIC 'q'

/* ioctls and their arguments */
#define EQUALIZER_WRITE_DIGIT _IOW(EQUALIZER_MAGIC, 1, u8 *)

#endif
