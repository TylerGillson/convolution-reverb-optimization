/**
 * WAVE file header format
 */
 
#ifndef WAVE_H
#define WAVE_H

typedef struct WaveHeader {
    // Riff Wave Header
    char chunkId[4];			// "RIFF"
    int  chunkSize;				// Size of remaining file in bytes (36 + Subchunk2Size)
    char format[4];				// "WAVE"

    // Format Subchunk
    char subChunk1Id[4];		// "fmt " (trailing null char)
    int  subChunk1Size;			// Length of remaining format data
    short int audioFormat;		// Format type (1 = PCM, 3 = IEEE float, 6 = 8-bit A law, 7 = 8-bit mu law)
    short int numChannels;		// Number of channels (1 = Mono, 2 = Stereo)
    int sampleRate;				// Sample rate (blocks per second)
    int byteRate;				// SampleRate * NumChannels * BitsPerSample/8
    short int blockAlign;		// Frame size in bytes (NumChannels * BitsPerSample/8)
    short int bitsPerSample;	// Sample size in bits (e.g. 8, 16, etc.)
    //short int extraParamSize;

    // Data Subchunk
    char subChunk2Id[4];		// "data" or "FLLR"
    int  subChunk2Size;			// Size of sound sample data in bytes (NumSamples * NumChannels * BitsPerSample/8)

} WaveHeader;

typedef struct WaveData {
	int length;
	double *sampleData;
} WaveData;

#endif
