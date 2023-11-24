/* WAVE sound file writer for recording 16-bit output during program development */

#ifndef WAVE_WRITER_H
#define WAVE_WRITER_H

#ifdef __cplusplus
	extern "C" {
#endif
#include <stdio.h>

FILE * wave_open( long sample_rate, const char* filename );
void wave_enable_stereo( void );
bool wave_write( short const* in, int count );
long wave_sample_count( void );
void wave_write_header( void );
void wave_close( void );
void wave_set_8bit(void);
void wave_disable_header(void);

#ifdef __cplusplus
	}
#endif

#endif
