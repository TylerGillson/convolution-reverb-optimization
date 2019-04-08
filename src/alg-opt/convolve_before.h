#include "float.h"
#include "wave_utils.c"
#include "fft.c"

#define TRUE 1
#define FALSE 0

int N, M, P, i;
double elapsed, max;
double *Y;
clock_t before;
WaveData X, H;

/**
 * Extract sample data from dry recording and impulse response audio files:
 */
void initialize(char * inputFile, char * irFile, int verbose) {
	if (verbose == TRUE)
		printf("\nReading dry sound and impulse response files ...\n\n");
	X = read_wav(inputFile, verbose);	
	H = read_wav(irFile, verbose);
	if (verbose == TRUE) printf("Done!\n\n");
}

/**
 * Input-side convolution algorithm.
 */ 
void convolve_input_side() {
	// Zero the output array:
	for (i = 0; i < P; i++)
		Y[i] = 0.0;
	
	// Perform the convolution:
	max = DBL_MIN;
	for (i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			Y[i+j] += X.sampleData[i] * H.sampleData[j];
			
			// Determine the convolved audio's maximum absolute value:
			double abs_val = fabs(Y[i+j]);
			if (abs_val > max)
				max = abs_val;
		}
	}
}

/**
 * Convolve the sample data from the input and the impulse response files;
 * normalize the resulting convolved audio data, then write it to disk as
 * a new WAVE file at the specified filepath.
 */
void convolve(char * outputFile, int verbose) {
	// Determine size of Y[]:
	N = X.length;
	M = H.length;
	P = N + M - 1;
	
	// Allocate space for the convolution data:
	Y = (double *)malloc(sizeof(double)*P);
	if (Y == NULL) {
		printf("malloc of size %d failed!\n", P);
		return;
	}
	
	// Convolve the input and impulse response sample data:
	if (verbose == TRUE) printf("Beginning convolution ...\n");
	convolve_input_side();
	if (verbose == TRUE) printf("Successfully performed convolution.\n\n");
	
	// Normalize convolved audio data:
	if (verbose == TRUE) printf("Normalizing convolved audio ...\n");
	for (i = 0; i < P; i++)
		Y[i] /= max;
	if (verbose == TRUE) printf("Done!\n\n");
	
	// Write convolved data to a new .wav file:
	if (verbose == TRUE) printf("Creating output file ...\n");
	write_wav(outputFile, Y, P, 1, verbose);
	if (verbose == TRUE) printf("Done!\n");
}
