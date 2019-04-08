#define FREQUENCY_CONVOLVE(rex,refr,imx,imfr,j)\
		temp=(rex[j]*refr[j])-(imx[j]*imfr[j]);\
		imx[j]=(rex[j]*imfr[j])+(imx[j]*refr[j]);\
		rex[j]=temp

// Before (convolve.h lines 238-243):
	
	// Multiply the frequency spectrum by the frequency response:
	for (int j = 0; j < spectra_len; j++) {
		temp   = (REX[j] * REFR[j]) - (IMX[j] * IMFR[j]);
		IMX[j] = (REX[j] * IMFR[j]) + (IMX[j] * REFR[j]);
		REX[j] = temp;			
	}

// After (convolve.h lines 243-255):

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
