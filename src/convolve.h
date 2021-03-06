#include "float.h"
#include "wave_utils.c"
#include "fft.c"

#define TRUE 1
#define FALSE 0

#define FREQUENCY_CONVOLVE(rex,refr,imx,imfr,j)\
		temp=(rex[j]*refr[j])-(imx[j]*imfr[j]);\
		imx[j]=(rex[j]*imfr[j])+(imx[j]*refr[j]);\
		rex[j]=temp

int N, M, P, i;
double elapsed, max = DBL_MIN;
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
 * Load the next segment of the input file's sample data
 * into XX, zero-padding all entries to the right of the
 * filter kernel, then shift the sliding window over.
 */
int slide_window(int xx_len, int segment_len, int window_idx, double *XX) {
	// For-loop unrolled once for hand tuning #2
	for (int i = 0; i < xx_len-1; i += 2) {
		int in_range = (i + 1 < segment_len) ? TRUE : FALSE;
		if (in_range) {
			XX[i]   = X.sampleData[window_idx + i];
			XX[i+1] = X.sampleData[window_idx + i + 1];
		}
		else {
			XX[i]   = 0.0;
			XX[i+1] = 0.0;
		}	
	}
	if (i == xx_len-1)
		XX[i] = 0.0;
		
	window_idx += segment_len;
	return window_idx;
}

/**
 * Perform pre-processing for the IFFT: i.e., combine real and
 * imaginary components of frequency response into XX.
 */
void pre_process_fft(int spectra_len, double *XX, double *REX, double *IMX) {
	// idx pre-calculated to minimize work inside loop for hand tuning #1
	int idx;
	for (int i = 0; i < spectra_len; i++) {
		idx = i * 2;
		XX[idx]   = REX[i];
		XX[idx+1] = IMX[i];
	}
}

/**
 * Extract real & imaginary components from frequency response into REX & IMX.
 */
void post_process_fft(int fft_len, double *XX, double *REX, double *IMX) {
	int idx;
	for (int i = 0; i < fft_len-1; i += 2) {
		// idx pre-calculated to minimize work inside loop for hand tuning #1
		idx = i / 2;
		REX[idx] = XX[i];
		IMX[idx] = XX[i+1];

		// Zero-out XX as it is copied into REX & IMX:
		XX[i]    = 0.0;
		XX[i+1]  = 0.0;
	}
}


/**
 * Extract real & imaginary components from frequency response into
 * REX, IMX, REFR, and IMFR. Called once, prior to the main body of
 * the convolution algorithm.
 */
void post_process_fft_one_off(int fft_len, double *XX,
							  double *REX, double *IMX,
							  double *REFR, double *IMFR) {
	int idx;
	for (int i = 0; i < fft_len-1; i += 2) {
		idx = i / 2;
		REX[idx] = XX[i];
		IMX[idx] = XX[i+1];
	
		// Copy values to REFR & IMFR (hand tuning #4):
		REFR[idx] = REX[idx];
		IMFR[idx] = IMX[idx];
		
		// Zero-out XX as it is copied into REX & IMX:
		XX[i]    = 0.0;
		XX[i+1]  = 0.0;
	}
}


/**
 * Used to determine the convolved audio's maximum absolute value.
 * Method added for hand tuning #3.
 */
void update_max(double val) {
	double abs_val = fabs(val);
	if (abs_val > max)
		max = abs_val;
}

/**
 * Overlap-add FFT convolution algorithm.
 */
void convolve_overlap_add_fft() {
	
	// Rename variables for clarity:
	int num_points = X.length;
	int filter_kernel_len = H.length;
	
	// Get smallest power of 2 larger than filter_kernel_len:
	int fft_len = 2;
	while (fft_len < filter_kernel_len)
		fft_len *= 2;
	
	// Determine segment length:
	int segment_len = (fft_len + 1) - filter_kernel_len;
	
	// Ensure that input sample data array is an even multiple of segment_len:
	if (num_points % segment_len != 0) {
		double *tmp;
		int new_len = ((num_points / segment_len) + 1) * segment_len;
		int points_diff = new_len - num_points;
		num_points += points_diff;
		P += points_diff;
		
		// Increase size of input sample data array:
		tmp = realloc(X.sampleData, sizeof(double) * num_points);
		if (tmp == NULL) { 
			printf("realloc failed while increasing X.sampleData!\n");
			return;
		} else
			X.sampleData = tmp;
		
		// Zero out added entries:
		for (i = num_points-points_diff; i < num_points; i++)
			X.sampleData[i] = 0.0;
		
		// Increase size of output array to match:
		tmp = realloc(Y, sizeof(double) * P);
		if (tmp == NULL) { 
			printf("realloc failed while increasing output array Y[]!\n");
			return;
		} else
			Y = tmp;
	}
	
	// Determine number of segments:
	int num_segments = num_points / segment_len;
	
	// Determine array sizes:
	int xx_len = fft_len * 2;
	int spectra_len = fft_len / 2;
	int olap_len = filter_kernel_len - 1;
	
	// Initialize arrays:
	double *XX = (double *)malloc(sizeof(double) * xx_len);
	double *REX = (double *)malloc(sizeof(double) * spectra_len);
	double *IMX = (double *)malloc(sizeof(double) * spectra_len);
	double *REFR = (double *)malloc(sizeof(double) * spectra_len);
	double *IMFR = (double *)malloc(sizeof(double) * spectra_len);
	double *OLAP = (double *)malloc(sizeof(double) * olap_len);
	
	// Ensure initialization worked:
	if (XX == NULL || REX == NULL || IMX == NULL || 
		REFR == NULL || IMFR == NULL || OLAP == NULL) {
		printf("malloc failed while initializing arrays!\n");
		return;
	}
	
	// Zero the overlapping sample array:
	for (i = 0; i < olap_len; i++)
		OLAP[i] = 0.0;
	
	// Load the filter kernel (impulse response) into XX,
	// zero-padding all entries to the right of the filter kernel.
	for (i = 0; i < xx_len; i++)
		XX[i] = (i < filter_kernel_len) ? H.sampleData[i] : 0.0;
	
	// Perform the FFT on XX, then split the result
	// into the spectra arrays:
	four1(XX-1, fft_len, 1);
	
	// Save the frequency response into REFR & IMFR during post-processing:
	post_process_fft_one_off(fft_len, XX, REX, IMX, REFR, IMFR);
	
	// Process each of the segments:
	int j;
	int window_idx = 0, output_idx = 0;
	double temp;
	
	for (i = 0; i < num_segments; i++) {
		// Load next segment of input sample data into XX:
		window_idx = slide_window(xx_len, segment_len, window_idx, XX);
		
		// Perform FFT on XX, then split the result
		// into the spectra arrays:
		four1(XX-1, fft_len, 1);
		post_process_fft(fft_len, XX, REX, IMX);
		
		// For-loop unrolled twice for hand tuning #5
		// Multiply the frequency spectrum by the frequency response:
		for (j = 0; j < spectra_len-2; j += 3) {
			FREQUENCY_CONVOLVE(REX, REFR, IMX, IMFR, j);
			FREQUENCY_CONVOLVE(REX, REFR, IMX, IMFR, j+1);
			FREQUENCY_CONVOLVE(REX, REFR, IMX, IMFR, j+2);
		}
		if (j == spectra_len - 2) {
			FREQUENCY_CONVOLVE(REX, REFR, IMX, IMFR, j);
			FREQUENCY_CONVOLVE(REX, REFR, IMX, IMFR, j+1);
		}
		else if (j == spectra_len - 1)
			FREQUENCY_CONVOLVE(REX, REFR, IMX, IMFR, j);
		
		// Put REX & IMX into XX, the perform the IFFT on XX:
		pre_process_fft(spectra_len, XX, REX, IMX);
		four1(XX-1, fft_len, -1);
		
		// Add the last segment's overlap to this segment:
		for (j = 0; j < olap_len; j++)
			XX[j] += OLAP[j];
		
		// Save the samples that will overlap the next segment:
		for (j = segment_len; j < fft_len; j++)
			OLAP[j-segment_len] = XX[j];
		
		// Output the segment samples stored in XX[0]-XX[segment_len-1]
		// to the output file's data array:
		for (j = 0; j < segment_len; j++) {
			update_max(XX[j]);
			Y[output_idx+j] = XX[j];
		}
		output_idx += segment_len;	
	}
	
	// Add all samples remaining in OLAP to the output file's data array:
	for (j = 0; j < olap_len; j++) {
		update_max(OLAP[j]);
		Y[output_idx+j] = OLAP[j];
	}
	
	// Clean up:
	free(XX);
	free(REX);
	free(IMX);
	free(REFR);
	free(IMFR);
	free(OLAP);
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
	//convolve_input_side();
	convolve_overlap_add_fft();
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
