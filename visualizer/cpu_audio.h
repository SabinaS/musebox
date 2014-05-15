#ifndef _CPU_AUDIO_H
#define _CPU_AUDIO_H

#include <linux/ioctl.h>

#define CPU_AUDIO_MAGIC 4108 * 11
#define SAMPLENUM 32768

#ifndef CPU_AUDIO_US
struct sample {
    s16 left;
    s16 right; 
};
#else
struct sample {
	int16_t left;
	int16_t right;
};
#endif

/* ioctls and their arguments */
#define CPU_AUDIO_READ_SAMPLES _IOR(CPU_AUDIO_MAGIC, 1, struct sample *) //writes to freq_spec.sv 
#define CPU_AUDIO_WRITE_SAMPLES _IOW(CPU_AUDIO_MAGIC, 1, struct sample *) //writes to freq_spec.sv 

#endif
