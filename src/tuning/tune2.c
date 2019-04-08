/**
 * Load the next segment of the input file's sample data
 * into XX, zero-padding all entries to the right of the
 * filter kernel, then shift the sliding window over.
 */
int slide_window(int xx_len, int segment_len, int window_idx, double *XX) {
	for (int i = 0; i < xx_len-1; i+=2) {
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
