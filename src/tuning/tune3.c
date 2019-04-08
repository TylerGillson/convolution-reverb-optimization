/**
 * Used to determine the convolved audio's maximum absolute value.
 */
void update_max(double val) {
	double abs_val = fabs(val);
	if (abs_val > max)
		max = abs_val;
}

// BEFORE (convolve.h 222-239):

	// Output the segment samples stored in XX[0]-XX[segment_len-1]
	// to the output file's data array:
	for (j = 0; j < segment_len; j++)
		Y[output_idx+j] = XX[j];
	output_idx += segment_len;	
}

// Add all samples remaining in OLAP to the output file's data array:
for (j = 0; j < olap_len; j++)
	Y[output_idx+j] = OLAP[j];

// Determine the convolved audio's maximum absolute value:
max = DBL_MIN;
for (i = 0; i < P; i++) {
	double abs_val = fabs(Y[i]);
	if (abs_val > max)
		max = abs_val;
}

// AFTER (convolve.h 234-247):

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
