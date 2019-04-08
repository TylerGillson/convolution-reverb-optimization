/**
 * Perform pre-processing for the IFFT: i.e., combine real and
 * imaginary components of frequency response into XX.
 */
void pre_process_fft(int spectra_len, double *XX, double *REX, double *IMX) {
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
		idx = i / 2;
		REX[idx] = XX[i];
		IMX[idx] = XX[i+1];
		
		// Zero-out XX as it is copied into REX & IMX:
		XX[i]    = 0.0;
		XX[i+1]  = 0.0;
	}
}
