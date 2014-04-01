#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memcmp
#include <stdint.h> // for int16_t and int32_t
#include <math.h>
#include "fft.h"

struct wavfile
{
	char    id[4];          // should always contain "RIFF"
	int32_t totallength;    // total file length minus 8
	char    wavefmt[8];     // should be "WAVEfmt "
	int32_t format;         // 16 for PCM format
	int16_t pcm;            // 1 for PCM format
	int16_t channels;       // channels
	int32_t frequency;      // sampling frequency
	int32_t bytes_per_second;
	int16_t bytes_by_capture;
	int16_t bits_per_sample;
	char    data[4];        // should always contain "data"
	int32_t bytes_in_data;
} __attribute__((__packed__));

int is_big_endian(void) {
	union {
		uint32_t i;
		char c[4];
	} bint = {0x01000000};
	return bint.c[0]==1;
}

int main(int argc, char *argv[]) {
	char *filename=argv[1];
	FILE *wav = fopen(filename,"rb");
	struct wavfile header;

	if ( wav == NULL ) {
		fprintf(stderr,"Can't open input file %s\n", filename);
		exit(1);
	}

	// read header
	if ( fread(&header,sizeof(header),1,wav) < 1 ) {
		fprintf(stderr,"Can't read input file header %s\n", filename);
		exit(1);
	}

	// if wav file isn't the same endianness than the current environment
	// we quit
	if ( is_big_endian() ) {
		if (   memcmp( header.id,"RIFX", 4) != 0 ) {
			fprintf(stderr,"ERROR: %s is not a big endian wav file\n", filename); 
			exit(1);
		}
	} else {
		if (   memcmp( header.id,"RIFF", 4) != 0 ) {
			fprintf(stderr,"ERROR: %s is not a little endian wav file\n", filename); 
			exit(1);
		}
	}

	if (   memcmp( header.wavefmt, "WAVEfmt ", 8) != 0 
		|| memcmp( header.data, "data", 4) != 0 
			) {
		fprintf(stderr,"ERROR: Not wav format\n"); 
		exit(1); 
	}
	if (header.format != 16) {
		fprintf(stderr,"\nERROR: not 16 bit wav format.");
		exit(1);
	}
	fprintf(stderr,"format: %d bits", header.format);
	if (header.format == 16) {
		fprintf(stderr,", PCM");
	} else {
		fprintf(stderr,", not PCM (%d)", header.format);
	}
	if (header.pcm == 1) {
		fprintf(stderr, " uncompressed" );
	} else {
		fprintf(stderr, " compressed" );
	}
	fprintf(stderr,", channel %d", header.pcm);
	fprintf(stderr,", freq %d", header.frequency );
	fprintf(stderr,", %d bytes per sec", header.bytes_per_second );
	fprintf(stderr,", %d bytes by capture", header.bytes_by_capture );
	fprintf(stderr,", %d bits per sample", header.bytes_by_capture );
	fprintf(stderr,", %d bytes in data", header.bytes_in_data );
	fprintf(stderr,"\n" );

	printf("\nchunk=%d\n",header.bytes_in_data/header.bytes_by_capture);
	//показатель двойки массива
	int p2=(int)(log2(header.bytes_in_data/header.bytes_by_capture));
	printf("log2=%d\n",p2);
	long int size_array=1<<(int)(log2(header.bytes_in_data/header.bytes_by_capture));
	printf("size array=%ld \n", size_array);
	if ( memcmp( header.data, "data", 4) != 0 ) { 
		fprintf(stderr,"ERROR: Prrroblem?\n"); 
		exit(1); 
	}
	fprintf(stderr,"wav format\n");



//*****************************************************//
	FILE * logfile;
	logfile = fopen("test.txt", "w+");
	if (!logfile)
	{
		printf("Failed open file, error\n");
		return 0;
	}
//*****************************************************//	
	
	//fft!!!
	float *c;  // массив поворотных множителей БПФ
	float *in; // входной массив
	float *out;// выходной массив
	
	c = calloc(size_array*2, sizeof(float));
	in = calloc(size_array*2, sizeof(float));
	out = calloc(size_array*2, sizeof(float));
	
	fft_make(p2,c);// функция расчёта поворотных множителей для БПФ
	
	int16_t value;
	int i=0;
	int j=0;
	
	while( fread(&value,sizeof(value),1,wav) ) {

		//fprintf(logfile,"%d %hi\n",i, value);
		in[j]=(float)value;
		i++;
		j+=2;
		if (i > size_array)  break;
	}
	//fft_calc(p2, c,	in,	out, 0);
	fft_calc(p2, c,	in,	out, 1);
	j=0;
	double delta=((float)header.frequency)/(float)size_array;
	//printf("delta=%3.10f",delta);
	double cur_freq=0;
	
	double * ampl;
	ampl = calloc(size_array*2, sizeof(double));
	
	double max;
	int start_max=0;
	int pos_max=0;
	//for(i=0;i<(2*size_array);i+=2) {
	for(i=0;i<(size_array);i+=2) {
		fprintf(logfile,"%.6f %f\n",cur_freq, (sqrt(out[i]*out[i]+out[i+1]*out[i+1])));
		ampl[i]=cur_freq; //freq
		ampl[i+1]=sqrt(out[i]*out[i]+out[i+1]*out[i+1]); //amp
		
		if(((ampl[i+1]-150) >0) && (!start_max)) {
			start_max = 1;
			max=ampl[i+1];
			pos_max=i;
		}
		
		if (start_max) {
			if((ampl[i+1]-100) >0) {
				if(ampl[i+1]>max) max=ampl[i+1];
				pos_max=i;
			} else {
				printf("Max Freq = %.3f , amp =%.3f\n",ampl[pos_max],ampl[pos_max+1]);
				start_max =0;
			}
		}
		
		
		cur_freq+=delta;
	}
	
	for(i=2;i<(2*size_array-2);i+=2) {
		//fprintf(logfile,"%.6f %f\n",ampl[i], (ampl[i-1]-ampl[i+3])/2); // df/dx
	}
	
	free(c);
	free(in);
	free(out);
	free(ampl);

	fclose(logfile);
	fclose(wav);
	exit(0);
}
