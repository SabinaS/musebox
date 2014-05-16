#include <stdio.h>
#include <math.h>
#include <complex.h>									//This library is declared before fftw3.h
#include <fftw3.h>
#include <stdint.h>

#define CPU_AUDIO_US
#include "../cpu_audio.h"

#define SAMPLE_RATE 44100

static int slot_values[12] = {31, 72, 150, 250, 440, 630, 1000, 2500, 5000, 8000, 14000, 20000};

int main(void)
{
	int i;
	int Npoints = SAMPLENUM;
	fftw_complex *in, *out, *reverse;
	fftw_plan plan, reverse_plan;

	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);			//allocating memory
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);		//allocating memory
	reverse = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);
	plan = fftw_plan_dft_1d(Npoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 	//Here we set which kind of transformation we want to perform
	reverse_plan = fftw_plan_dft_1d(Npoints, out, reverse, FFTW_BACKWARD, FFTW_ESTIMATE);

	printf("\nCoefficcients of the expansion:\n\n");
	for(i = 0; i < Npoints; i++)
	{
		in[i] = i + 0 * I;
		// printf("%d %11.7f %11.7f\n", i, creal(in[i]), cimag(in[i]));		//creal and cimag are functions of complex.h 
	}
	printf("\n");

	fftw_execute(plan); 								//Execution of FFT

	printf("Output Amplitude:\n\n");
	for(i = 0; i < Npoints; i++)
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
	return 0;
}
