#include <stdio.h>
#include <math.h>
#include <complex.h>									//This library is declared before fftw3.h
#include <fftw3.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define CPU_AUDIO_US
#include "../cpu_audio.h"
#include "../visualizer_driver.h"

#define SAMPLE_RATE 44100

static int slot_values[12] = {31, 72, 150, 250, 440, 630, 1000, 2500, 5000, 8000, 14000, 20000};

int main(void)
{
	int i;
	int Npoints = SAMPLENUM;
	fftw_complex *in, *out, *reverse;
	fftw_plan plan, reverse_plan;
    char *file = "/dev/visualizer";
    char *audio = "/dev/cpu_audio";
    int frec_spec_fd, box_fd;
    struct sample samples[SAMPLENUM];

    if ((frec_spec_fd = open(file, O_RDWR)) == -1 ) {
        fprintf(stderr, "could not open %s\n", file);
        return -1;
    }
    if ((box_fd = open(audio, O_RDWR)) == -1 ) {
        fprintf(stderr, "could not open %s\n", audio);
        return -1;
    }

	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);			//allocating memory
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);		//allocating memory
	reverse = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);
	plan = fftw_plan_dft_1d(Npoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 	//Here we set which kind of transformation we want to perform
	reverse_plan = fftw_plan_dft_1d(Npoints, out, reverse, FFTW_BACKWARD, FFTW_ESTIMATE);

    while (ioctl(box_fd, CPU_AUDIO_READ_SAMPLES, samples)) {
        if (errno == EAGAIN) {
           // nanosleep(&duration, &remaining);
           continue;
        }
        fprintf(stderr, "errno: %d\n", errno);
        perror("ioctl read failed!");
        close(box_fd);
        return -1;
    }
	printf("\nCoefficcients of the expansion:\n\n");
	for(i = 0; i < Npoints; i++)
	{
		in[i] = samples[i].left + 0 * I;
		// printf("%d %11.7f %11.7f\n", i, creal(in[i]), cimag(in[i]));		//creal and cimag are functions of complex.h 
	}
	printf("\n");

	fftw_execute(plan); 								//Execution of FFT

	printf("Output Amplitude:\n\n");
	for(i = 2 * Npoints / 4; i < 3 * Npoints / 4; i++)
	{
		printf("%d %f\n", i, 10 * log(creal(out[i]) * creal(out[i]) + cimag(out[i]) * cimag(out[i])));
	}

	fftw_execute(reverse_plan); 								//Execution of FFT

	printf("Reverse output:\n\n");
	for(i = 0; i < Npoints; i++)
	{
		// printf("%d %11.7f %11.7f\n", i, creal(reverse[i]) / Npoints, cimag(reverse[i]));
	}

	
	fftw_destroy_plan(plan);							//Destroy plan
	fftw_free(in);			 						//Free memory
	fftw_free(out);			 						//Free memory
	fftw_destroy_plan(reverse_plan);							//Destroy plan
	fftw_free(reverse);			 						//Free memory
    close(frec_spec_fd);
    close(box_fd);
	return 0;
}
