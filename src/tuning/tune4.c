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

// Before (convolve.h lines 193-199):
	
	post_process_fft(fft_len, XX, REX, IMX);
	
	// Save the frequency response into REFR & IMFR:
	for (i = 0; i < spectra_len; i++) {
		REFR[i] = REX[i];
		IMFR[i] = IMX[i];
	}

// After (convolve.h lines 221-221):
	
	// Save the frequency response into REFR & IMFR during post-processing:
	post_process_fft_one_off(fft_len, XX, REX, IMX, REFR, IMFR);
