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
#include <unistd.h>
#include <pthread.h>
#include <float.h>

#define CPU_AUDIO_US
#include "../cpu_audio.h"
#include "../visualizer_driver.h"

#define SAMPLE_RATE 44100
#define MAX_AMP 190
#define MAX_HEIGHT 479

static int slot_values[14] = {0, 31, 72, 150, 250, 440, 630, 1000, 2500, 5000, 8000, 14000, 20000, 22050};
static volatile double equalizer_multiples[14] = {1, 1, 1, 1, 1, 1, 0.125, 0.125, 0.125, 1, 1, 1, 1, 1};
static double slot_heights[12];
static freq_slot slots[12];

int main(void)
{
	int i, j;
	int Npoints = SAMPLENUM;
	fftw_complex *in, *out, *reverse;
	fftw_plan plan, reverse_plan;
	char *file = "/dev/visualizer";
	char *audio = "/dev/cpu_audio";
	int frec_spec_fd, box_fd;
	struct sample samples[SAMPLENUM];
	struct sample samples_out[SAMPLENUM];

	if ((frec_spec_fd = open(file, O_RDWR)) == -1 ) {
		fprintf(stderr, "could not open %s\n", file);
		return -1;
	}
	if ((box_fd = open(audio, O_RDWR)) == -1 ) {
		close(frec_spec_fd);
		fprintf(stderr, "could not open %s\n", audio);
		return -1;
	}

	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);			//allocating memory
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);		//allocating memory
	reverse = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*Npoints);
	plan = fftw_plan_dft_1d(Npoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 	//Here we set which kind of transformation we want to perform
	reverse_plan = fftw_plan_dft_1d(Npoints, out, reverse, FFTW_BACKWARD, FFTW_ESTIMATE);

	while (1) {
	while (ioctl(box_fd, CPU_AUDIO_READ_SAMPLES, samples)) {
		if (errno == EAGAIN) {
		   // nanosleep(&duration, &remaining);
		   continue;
		}
		fprintf(stderr, "errno: %d\n", errno);
		perror("ioctl read failed!");
		close(box_fd);
		close(frec_spec_fd);
		return -1;
	}
	// printf("\nCoefficcients of the expansion:\n\n");
	for(i = 0; i < Npoints; i++)
	{
		in[i] = samples[i].left + 0 * I;
		// printf("%d %11.7f %11.7f\n", i, creal(in[i]), cimag(in[i]));		//creal and cimag are functions of complex.h 
	}
	// printf("\n");

	fftw_execute(plan); 								//Execution of FFT

	// The equalizer is roughly equal in computation to the visualizer
	for(i = 1; i < 13; i++) {
		// Each slot computes the effect of it and the previous slot on a bin.
		// Determine the bin ranges
		int lowerBin = SAMPLENUM * slot_values[i - 1] / SAMPLE_RATE;
		int centerBin = SAMPLENUM * slot_values[i] / SAMPLE_RATE;
		int lowerRange = centerBin - lowerBin;
		// Consider bins contained withing your slot and the previous slot
		for (j = lowerBin; j < centerBin; j++) {
			// Compute the weighted average of the equalizer applied to this bin
			// Figure out the percentage difference from this bin to the previous
			double prevDiff = (j - lowerBin) / ((double) lowerRange);
			// Figure out the percentage difference between this bin and the next
			double currDiff = (centerBin - j) / ((double) lowerRange);
			// Figure out the gain due to the previous slot
			double prevGain = prevDiff * equalizer_multiples[i - 1];
			// Figure out the gain due to the current slot
			double currGain = currDiff * equalizer_multiples[i];
			// Add them for the overall gain
			double gain = currGain + prevGain;
			// Multiply the real and imaginary component by this value
			// if (j % 100 == 0)
			// 	printf("Old freq %d, real %f, imag %f\n", SAMPLE_RATE * j / SAMPLENUM, creal(out[j]), cimag(out[j]));
			out[j] = creal(out[j]) * gain + cimag(out[j]) * gain * I;
			// Assign it to the conjugate as well
			out[SAMPLENUM - j - 1] = creal(out[j]) * gain + cimag(out[j]) * gain * I;
			// if (j % 100 == 0)
			// 	printf("New bin %d, real %f, imag %f\n", SAMPLE_RATE * j / SAMPLENUM, creal(out[j]), cimag(out[j]));
		}
		// printf("Slot %d: %f\n", slot_values[i], amp);
	}
	// printf("Output Amplitude:\n\n");
	// For each slot
	for(i = 1; i < 13; i++) {
		double amp = 0;
		// Determine the bin ranges
		int lowerBin = SAMPLENUM * slot_values[i - 1] / SAMPLE_RATE;
		int centerBin = SAMPLENUM * slot_values[i] / SAMPLE_RATE;
		int upperBin = SAMPLENUM * slot_values[i + 1] / SAMPLE_RATE;
		int totalRange = upperBin - lowerBin;
		int lowerRange = centerBin - lowerBin;
		int upperRange = upperRange - centerBin;
		// Consider bins contained within it and the two slots next to it
		for (j = lowerBin; j <= upperBin; j++) {
			// Compute teh amplitude of this bin
			double realMag = creal(out[i]) / Npoints;
			double imagMag = cimag(out[i]) / Npoints;
			double magn = 10 * log(realMag * realMag + imagMag * imagMag);
			// Scale it according to its position
				// First, apply the slope of its side
			if (j < centerBin) {
				magn = magn * (((double) j) - lowerBin) / lowerRange;
			}
			else if (j > centerBin) {
				// First, apply the slope of its side
				magn = magn * (upperBin - ((double) j)) / upperRange;
			}
			// Then, scale it by the total number of elements
			magn = magn / totalRange;
			// Finally, add it to the slot amplitude
			amp += magn;
		}
		// Assign this slot amplitude
		slot_heights[i - 1] = amp;
		// printf("Slot %d: %f\n", slot_values[i], amp);
	}

	int height;
	// For each slot
	for(i = 0; i < 12; i++) {
		// Its height is just the difference from the maximum amplitude, but we need to make it more dynamic
		height = MAX_HEIGHT - (int) fmin((MAX_AMP - slot_heights[i]) * slot_heights[i] / 9, MAX_HEIGHT);
		// printf("Slot %d assigned height %d\n", slot_values[i + 1], height);
		slots[i].height = height;
		slots[i].addr = i; 
        if (ioctl(frec_spec_fd, VISUALIZER_WRITE_FREQ, &slots[i])) {
            perror("ioctl write failed!");
			close(box_fd);
            close(frec_spec_fd);
            return -1;
        }
	}
	// double maxDb = DBL_MIN;
	// for(i = 0; i < Npoints; i++)
	// {
	// 	double realMag = creal(out[i]) / Npoints;
	// 	double imagMag = cimag(out[i]) / Npoints;
	// 	double curMax = 10 * log(realMag * realMag + imagMag * imagMag);
	// 	// printf("%d %f\n", i, curMax);
	// 	if (curMax > maxDb)
	// 		maxDb = curMax;
	// }
	// printf("%f\n", maxDb);

	fftw_execute(reverse_plan); 								//Execution of FFT

	// printf("Reverse output:\n\n");
	for(i = 0; i < Npoints; i++)
	{
		// printf("%d %11.7f %11.7f\n", i, creal(reverse[i]) / Npoints, cimag(reverse[i]));
		samples_out[i].left = creal(reverse[i]) / Npoints;
		samples_out[i].right = creal(reverse[i]) / Npoints;
	}
	// Play the audio back
	if (ioctl(box_fd, CPU_AUDIO_WRITE_SAMPLES, samples_out)) {
		perror("ioctl write failed!");
		close(box_fd);
		close(frec_spec_fd);
		return -1;
	}
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
