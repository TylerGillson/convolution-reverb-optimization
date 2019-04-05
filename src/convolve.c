#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wave_utils.c"

double elapsed;
double *Y;
clock_t before;
WaveData X, H;			

/**
 * Extract sample data from dry recording and impulse response audio files:
 */
void initialize(char * inputFile, char * irFile) {
	printf("\nReading data from dry sound and impulse response files ...\n\n");
	X = read_wav(inputFile);	
	H = read_wav(irFile);
	printf("Done!\n\n");
}

/**
 * Convolve the sample data from the input and the impulse response files;
 * normalize the resulting convolved audio data, then write it to disk as a
 * new WAVE file at the specified filepath.
 */
void convolve(char * outputFile) {
	// Determine size of Y[]:
	int N = X.length;
	int M = H.length;
	int P = N + M - 1;
	
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
	printf("Beginning convolution ...\n");
	double max = 0.0;
	for (i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			Y[i+j] += X.sampleData[i] * H.sampleData[j];
			
			// Determine the convolved audio's maximum value:
			if (Y[i+j] > max)
				max = Y[i+j];
		}
	}
	printf("Successfully performed convolution.\n\n");
	
	// Normalize convolved audio data:
	printf("Normalizing convolved audio ...\n");
	for (i = 0; i < P; i++)
		Y[i] /= max;
	printf("Done!\n\n");
	
	// Write convolved data to a new .wav file:
	printf("Creating output file ...\n");
	write_wav(outputFile, Y, P, 1);
	printf("Done!\n");
}

/**
 * Given filepaths to a dry audio recording, an impulse response file,
 * and an output location, convolve the dry audio with the impulse response
 * and write the convolved audio data to disk at the specified location.
 */
int main(int argc, char **argv) {
	// Start timer:
	before = clock();
	
	// Ensure proper usage:
	if (argc < 3) {
		printf("Usage: convolve [inputFile] [irFile] [outputFile]");
		return -1;
	}
	
	// Extract command line args:
	char * inputFile = argv[1];
	char * irFile = argv[2];
	char * outputFile = argv[3];
	
	// Extract .wav data from input and impulse response files:
	initialize(inputFile, irFile);
	
	// Perform convolution and write convolved data to disk:
	convolve(outputFile);
	
	// Stop timer and report:
	elapsed = clock() - before;
	float seconds = elapsed / CLOCKS_PER_SEC;
	int minutes = seconds / 60;
    printf("\nElapsed [mm:ss]: %d:%.3f\n", minutes, seconds - (minutes * 60));
    
	// Clean up and exit:
	free(X.sampleData);
	free(H.sampleData);
	free(Y);
	return 0;
}


