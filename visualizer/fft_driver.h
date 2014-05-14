#ifndef _FFT_DRIVER_H
#define _FFT_DRIVER_H

#define SAMPLENUM 8192

#include <linux/ioctl.h>

#ifndef USER_SPACE_FFT
typedef struct {
	s16 real;
	s16 imag;
} complex_num;
#endif

#define FFT_DRIVER_MAGIC 8417

/* ioctls and their arguments */
#define FFT_DRIVER_READ_TRANSFORM  _IOWR(FFT_DRIVER_MAGIC, 2, struct complex_num *)

#endif

