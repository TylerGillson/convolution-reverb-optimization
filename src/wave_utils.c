/**
 * Utility methods for:
 *     1) Reading the contents of a .wav file and returning a WaveData struct.
 *     2) Writing the contents of an array containing convolved audio samples
 *        to disk.
 * 
 * Source for read_wav:  https://ubuntuforums.org/showthread.php?t=968690
 * Source for write_wav: http://www.mega-nerd.com/libsndfile/api.html#write
 */

#include <stdlib.h>
#include <sndfile.h>

#define TRUE 1
#define FALSE 0

typedef struct WaveData {
	int length;
	double *sampleData;
} WaveData;

/**
 * Read the contents of a .wav file and return a WaveData struct.
 */
WaveData read_wav(char * filepath, int verbose) {
	SNDFILE *sf;
    SF_INFO info;
    int num, num_samples;
    double *buff;
    int f, sr, c;
    
    // Initialize WaveData struct:
	WaveData wave_data;
	wave_data.length = -1;
    
    // Open the WAVE file:
    info.format = 0;
    sf = sf_open(filepath, SFM_READ, &info);
    if (sf == NULL) {
		printf("Failed to open the file.\n");
		return wave_data;
	}
	
    // Extract some of the header info to figure out how much data to read:
    f = info.frames;
    sr = info.samplerate;
    c = info.channels;
    num_samples = f*c;
    
    if (verbose == TRUE) {
		printf("frames=%d\n", f);
		printf("samplerate=%d\n", sr);
		printf("channels=%d\n", c);
		printf("num_samples=%d\n", num_samples);
	}
	
	// Allocate space for the sample data, then load it:
    buff = (double *) malloc(num_samples * sizeof(double));
    num = sf_read_double(sf, buff, num_samples);
    sf_close(sf);
    
    if (verbose == TRUE) printf("samples read: %d\n\n", num);
	
	// Update WaveData struct and return:
	wave_data.length = num_samples;
	wave_data.sampleData = buff;
	return wave_data;
}

/**
 * Write the contents of an array containing convolved audio samples to disk.
 */
void write_wav(char * filename, double * sample_data, int num_samples, int num_channels, int verbose) {
	SNDFILE *sf;
	SF_INFO info;
	sf_count_t written;
	
	// Prepare the SF_INFO struct:
	info.samplerate = 44100;
	info.channels = num_channels;
	info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
	
	// Ensure SF_INFO is properly formatted:
	if (sf_format_check(&info) != TRUE)
		printf("Error: invalid SF_INFO struct.\n");
	
	// Open for writing:
	sf = sf_open(filename, SFM_WRITE, &info);
	
	// Write data to disk and clean up:
	written = sf_write_double(sf, sample_data, num_samples);
	if (verbose == TRUE) printf("Created new .wav file with %lld samples.\n", written);
	sf_close(sf);
}
