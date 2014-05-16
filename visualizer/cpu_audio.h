#ifndef _CPU_AUDIO_H
#define _CPU_AUDIO_H

#include <linux/ioctl.h>

#define CPU_AUDIO_MAGIC 'r' * 7
#define SAMPLENUM 8192
#define BUFFER_SIZE 65535

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
#define CPU_AUDIO_WRITE_SAMPLES _IOWR(CPU_AUDIO_MAGIC, 2, struct sample *) //writes to freq_spec.sv 

#endif
