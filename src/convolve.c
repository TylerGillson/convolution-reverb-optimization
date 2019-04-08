#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "convolve.h"

/**
 * Given filepaths to a dry audio recording, an impulse response file,
 * and an output location, convolve the dry audio with the impulse response
 * and write the convolved audio data to disk at the specified location.
 * 
 * Compile with:
 *     gcc convolve.c -lsndfile -o convolve
 * 
 * Run with:
 *     ./convolve [inputFile] [irFile] [outputFile]
 * 
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
	initialize(inputFile, irFile, 1);
	
	// Perform convolution and write convolved data to disk:
	convolve(outputFile, 1);
	
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
