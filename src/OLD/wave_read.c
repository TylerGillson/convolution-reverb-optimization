/**
 * Parse header and sample data from a .wav file.
 * Source: http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
 **/
 
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "wave.h"

#define TRUE 1 
#define FALSE 0
#define DEBUG FALSE
#define VERBOSE FALSE

// Globals:
FILE *ptr;
unsigned char buffer4[4];
unsigned char buffer2[2];
char* filename;
char* seconds_to_time(float seconds);

int to_big_endian(unsigned char buff[4]) {
	return buff[0] | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24);
}

void get_format_name(int type_code, char format_name[10]) {
	switch (type_code) {
		case 1:
			strcpy(format_name, "PCM");
			break;
		case 6:
			strcpy(format_name, "A-law");
			break;
		case 7:
			strcpy(format_name, "Mu-law");
			break;
	}
}

/**
 * Extracts all information from a .wav file into
 * WaveHeader and WaveData structs. Returns the WaveData struct.
 * 
 * Note: Caller is responsible for freeing the memory associated with
 * the WaveData struct's sampleData array.
 */
WaveData extract_wav(char * filename) {
	WaveHeader waveHeader;
	WaveData waveData;
	waveData.length = -1;
		
	if (DEBUG == TRUE) printf("Opening file ...\n\n");
	ptr = fopen(filename, "rb");
	if (!ptr) {
		printf("Error opening file\n");
		return waveData;
	}
	
	// Read Header contents ...
	int read = 0;
	char format_name[10] = "";
	
	// ChunkID:
	read = fread(waveHeader.chunkId, sizeof(waveHeader.chunkId), 1, ptr);
	
	// ChunkSize:
	read = fread(buffer4, sizeof(buffer4), 1, ptr);
	waveHeader.chunkSize = to_big_endian(buffer4);
	
	// Format:
	read = fread(waveHeader.format, sizeof(waveHeader.format), 1, ptr);
	
	// Subchunk1ID:
	read = fread(waveHeader.subChunk1Id, sizeof(waveHeader.subChunk1Id), 1, ptr);
	
	// Subchunk1Size:
	read = fread(buffer4, sizeof(buffer4), 1, ptr);
	waveHeader.subChunk1Size = to_big_endian(buffer4);
	
	// AudioFormat:
	read = fread(buffer2, sizeof(buffer2), 1, ptr);
	waveHeader.audioFormat = buffer2[0] | (buffer2[1] << 8);
	get_format_name(waveHeader.audioFormat, format_name);
	
	// NumChannels:
	read = fread(buffer2, sizeof(buffer2), 1, ptr);
	waveHeader.numChannels = buffer2[0] | (buffer2[1] << 8);
	
	// SampleRate:
	read = fread(buffer4, sizeof(buffer4), 1, ptr);
	waveHeader.sampleRate = to_big_endian(buffer4);
	
	// ByteRate:
	read = fread(buffer4, sizeof(buffer4), 1, ptr);
	waveHeader.byteRate = to_big_endian(buffer4);
	
	// BlockAlign:
	read = fread(buffer2, sizeof(buffer2), 1, ptr);
	waveHeader.blockAlign = buffer2[0] | (buffer2[1] << 8);
	
	// BitsPerSample:
	read = fread(buffer2, sizeof(buffer2), 1, ptr);
	waveHeader.bitsPerSample = buffer2[0] | (buffer2[1] << 8);
	
	// Skip past empty bytes if file has an 18 byte format subchunk:
	if (waveHeader.subChunk1Size != 16)
		read = fread(buffer2, sizeof(buffer2), 1, ptr);
	
	// Subchunk2ID:
	read = fread(waveHeader.subChunk2Id, sizeof(waveHeader.subChunk2Id), 1, ptr);
	
	// Subchunk2Size:
	read = fread(buffer4, sizeof(buffer4), 1, ptr);
	waveHeader.subChunk2Size = to_big_endian(buffer4);
	
	// calculate no. of samples
	long size_of_each_sample = (waveHeader.numChannels * waveHeader.bitsPerSample) / 8;
	long num_samples = waveHeader.subChunk2Size / size_of_each_sample;
			
	// calculate duration of file
	float duration_in_seconds = (float) waveHeader.chunkSize / waveHeader.byteRate;
	
	// set struct length
	waveData.length = num_samples;
	
	// Optionally print header description:
	if (DEBUG == TRUE) {
		printf("ChunkID: %s\n", waveHeader.chunkId); 
		printf("ChunkSize: %u bytes\n", waveHeader.chunkSize);
		printf("Format: %s\n", waveHeader.format);
		printf("Subchunk1ID: %s\n", waveHeader.subChunk1Id);
		printf("Subchunk1Size: %u\n", waveHeader.subChunk1Size);
		printf("AudioFormat: %u = %s\n", waveHeader.audioFormat, format_name);
		printf("NumChannels: %u\n", waveHeader.numChannels);
		printf("SampleRate: %u\n", waveHeader.sampleRate);
		printf("ByteRate: %u\n", waveHeader.byteRate);
		printf("BlockAlign: %u\n", waveHeader.blockAlign);
		printf("BitsPerSample: %u\n", waveHeader.bitsPerSample);
		printf("Subchunk2ID: %s\n", waveHeader.subChunk2Id);
		printf("Subchunk2Size: %u bytes\n\n", waveHeader.subChunk2Size);
		
		printf("Number of samples: %lu\n", num_samples);
		printf("Size of each sample: %ld bytes\n", size_of_each_sample);
		printf("Approx. Duration in seconds = %f\n", duration_in_seconds);	
	}
	
	// read each sample from data chunk if PCM
	if (waveHeader.audioFormat == 1) {
		long i = 0;
		char data_buffer[size_of_each_sample];
		int  size_is_correct = TRUE;

		// make sure that the bytes-per-sample is completely divisible by num.of channels
		long bytes_in_each_channel = (size_of_each_sample / waveHeader.numChannels);
		if ((bytes_in_each_channel  * waveHeader.numChannels) != size_of_each_sample) {
			printf("Error: %ld x %ud <> %ld\n", bytes_in_each_channel, waveHeader.numChannels, size_of_each_sample);
			size_is_correct = FALSE;
		}

		if (size_is_correct) { 
			// the valid amplitude range for values based on the bits per sample
			long low_limit = 0l;
			long high_limit = 0l;

			switch (waveHeader.bitsPerSample) {
				case 8:
					low_limit = -128;
					high_limit = 127;
					break;
				case 16:
					low_limit = -32768;
					high_limit = 32767;
					break;
				case 32:
					low_limit = -2147483648;
					high_limit = 2147483647;
					break;
			}		
			if (DEBUG == TRUE) printf("Valid range for data values: %ld to %ld \n\n", low_limit, high_limit);
			
			// Allocate array of doubles to hold sample data:
			double *sample_data = (double *)malloc(sizeof(double)*num_samples);
			double data_point;
			
			for (i = 1; i <= num_samples; i++) {
				if (DEBUG == TRUE && VERBOSE == TRUE) printf("Sample %ld / %ld\n", i, num_samples);
				
				read = fread(data_buffer, sizeof(data_buffer), 1, ptr);
				if (read == 1) {
				
					// dump the data read
					unsigned int xchannels = 0;
					int data_in_channel = 0;
					
					for (xchannels = 0; xchannels < waveHeader.numChannels; xchannels++) {
						if (DEBUG == TRUE && VERBOSE == TRUE) printf("Channel #%d: ", (xchannels+1));
						
						// convert data from little endian to big endian based on bytes in each channel sample
						if (bytes_in_each_channel == 4) {
							data_in_channel = data_buffer[0] | 
								(data_buffer[1] << 8) | 
								(data_buffer[2] << 16) |
								(data_buffer[3] << 24);
						}
						else if (bytes_in_each_channel == 2)
							data_in_channel = (data_buffer[0] & 255) | (data_buffer[1] << 8);
						else if (bytes_in_each_channel == 1)
							data_in_channel = data_buffer[0];
						
						// check if value was in range
						if (data_in_channel < low_limit || data_in_channel > high_limit)
							printf("**value out of range\n");
							
						// DIVIDE BY HIGH LIMIT TO SCALE INTO RANGE: [-1.0, 1.0]
						data_point = (double)data_in_channel / high_limit;
						if (DEBUG == TRUE && VERBOSE == TRUE) printf("%f ", data_point);
						sample_data[i-1] = data_point;
					}
					
					if (DEBUG == TRUE && VERBOSE == TRUE) printf("\n");
				}
				else {
					if (ferror(ptr) != 0)
						printf("Error reading file. %d bytes\n", read);
					else if (feof(ptr) != 0)
						printf("End of file reached.\n");
					else
						printf("Unknown error ...\n");
					break;
				}
			}
			if (DEBUG == TRUE) printf("Data extracted, closing file..\n");
			fclose(ptr);
			
			// Update struct pointer and return
			waveData.sampleData = sample_data;
			return waveData;
		}
	}
	
	if (DEBUG == TRUE) printf("Non PCM, closing file..\n");
	fclose(ptr);
	return waveData;
}
