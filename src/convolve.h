#include "wave_utils.c"

#define TRUE 1
#define FALSE 0

int P;
double elapsed;
double *Y;
clock_t before;
WaveData X, H;

/**
 * Extract sample data from dry recording and impulse response audio files:
 */
void initialize(char * inputFile, char * irFile, int verbose) {
	if (verbose == TRUE) printf("\nReading data from dry sound and impulse response files ...\n\n");
	X = read_wav(inputFile, verbose);	
	H = read_wav(irFile, verbose);
	if (verbose == TRUE) printf("Done!\n\n");
}

/**
 * Convolve the sample data from the input and the impulse response files;
 * normalize the resulting convolved audio data, then write it to disk as a
 * new WAVE file at the specified filepath.
 */
void convolve(char * outputFile, int verbose) {
	// Determine size of Y[]:
	int N = X.length;
	int M = H.length;
	P = N + M - 1;
	
	// Allocate space for the convolution data:
	Y = (double *)malloc(sizeof(double)*P);
	if (Y == NULL) {
		printf("malloc of size %d failed!\n", P);
		return;
	}

	// Zero the output array:
	int i;
	for (i = 0; i < P; i++)
		Y[i] = 0.0;
	
	// Perform the convolution:
	if (verbose == TRUE) printf("Beginning convolution ...\n");
	double max = 0.0;
	for (i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			Y[i+j] += X.sampleData[i] * H.sampleData[j];
			
			// Determine the convolved audio's maximum value:
			if (Y[i+j] > max)
				max = Y[i+j];
		}
	}
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
