/* snes_spc 0.9.0. http://www.slack.net/~ant/ */

#include "wave_writer.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Copyright (C) 2003-2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

enum { buf_size = 32768 * 2 };
enum { header_size = 0x2C };

typedef short sample_t;

static unsigned char* buf;
static FILE* file;
static long  sample_count_;
static long  sample_rate_;
static long  buf_pos;
static int   chan_count;
static int   output_8bit;

static void exit_with_error( const char* str )
{
	printf( "Error: %s\n", str ); getchar();
	exit( EXIT_FAILURE );
}

FILE* wave_open( long sample_rate, const char* filename )
{
	sample_count_ = 0;
	sample_rate_  = sample_rate;
	buf_pos       = header_size;
	chan_count    = 1;
    output_8bit   = 0;
	
	buf = (unsigned char*) malloc( buf_size * sizeof *buf );
	if ( !buf )
		exit_with_error( "Out of memory" );

    if ( strcmp( filename, "-" ) == 0 )
        file = tmpfile();  
    else
	    file = fopen( filename, "wb" );
    if ( !file )
		exit_with_error( "Couldn't open WAVE file for writing" );
	
	setvbuf( file, 0, _IOFBF, 32 * 1024L );

    return file;
}

void wave_enable_stereo( void )
{
	chan_count = 2;
}

void wave_set_8bit( void )
{
    output_8bit = 1;
}


static void flush_()
{
	if ( buf_pos && !fwrite( buf, buf_pos, 1, file ) )
		exit_with_error( "Couldn't write WAVE data" );
	buf_pos = 0;
}

void wave_write(short const *in, int remain)
{
    sample_count_ += remain;

    while (remain)
    {
        if (buf_pos >= buf_size)
            flush_();

        unsigned char *p = &buf[buf_pos];
        // How many samples fit in the buffer:
        //long n = (buf_size - buf_pos) / sizeof(sample_t);
        int n = (buf_size - buf_pos);  // 8-bit samples
        if (!output_8bit) {
            n >>= 1; // 16-bit samples
        }
        if (n > remain)
            n = remain;
        remain -= n;

        if (!output_8bit)
        {
            // 16-bit
            /* convert to LSB first format */
            while (n--)
            {
                unsigned short s = *in++;
                *p++ = (unsigned char)s;
                *p++ = (unsigned char)(s >> 8);
            }
        }
        else
        {
            // 8-bit, sign flip 
            while (n--)
            {
                *p++ = (unsigned char)(*in++ >> 8) ^ 0x80;
            }
        }
        buf_pos = p - buf;
        // assert(buf_pos <= buf_size);
    }
}

long wave_sample_count( void )
{
	return sample_count_;
}

static void set_le32( void* p, unsigned long n )
{
	((unsigned char*) p) [0] = (unsigned char) n;
	((unsigned char*) p) [1] = (unsigned char) (n >> 8);
	((unsigned char*) p) [2] = (unsigned char) (n >> 16);
	((unsigned char*) p) [3] = (unsigned char) (n >> 24);
}

void wave_write_header( void )
{
	if ( file )
	{
        unsigned char bits_per_sample = output_8bit ? 8 : 16;
        unsigned int bytes_per_sample = bits_per_sample / 8;

		/* generate header */
		unsigned char h [header_size] =
		{
			'R','I','F','F',
			0,0,0,0,        /* length of rest of file */
			'W','A','V','E',
			'f','m','t',' ',
			0x10,0,0,0,     /* size of fmt chunk */
			1,0,            /* uncompressed format */
			0,0,            /* channel count */
			0,0,0,0,        /* sample rate */
			0,0,0,0,        /* bytes per second */
			0,0,            /* bytes per sample frame */
			bits_per_sample,0, /* bits per sample */
			'd','a','t','a',
			0,0,0,0,        /* size of sample data */
			/* ... */       /* sample data */
		};
		long ds = sample_count_ * bytes_per_sample;
		int frame_size = chan_count * bytes_per_sample;
		
		set_le32( h + 0x04, header_size - 8 + ds );
		h [0x16] = chan_count;
		set_le32( h + 0x18, sample_rate_ );
		set_le32( h + 0x1C, sample_rate_ * frame_size );
		h [0x20] = frame_size;
		set_le32( h + 0x28, ds );
		
		flush_();
		
		/* write header */
		if (fseek( file, 0, SEEK_SET ) == 0) {
			fwrite( h, sizeof h, 1, file );
        }
    }
}

void wave_close( void ) {
    if ( file ) {
		fclose( file );
		file = 0;
		free( buf );
		buf = 0;
    }
}
