/* C example that opens a game music file and records 10 seconds to "out.wav" */
#include "gme/gme.h"

#include "wave_writer.h" /* wave_ functions for writing sound file */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>

#include "zlib.h"

#define T_SEC_DEFAULT 30

void handle_error( const char* str );
void usage( void );




// Ungzip the infile to a temporary file, replace the 
// infile with the tmp file path.
bool unGzip(char* infile, char* inflatedFilename, bool verbose)
{
    bool success = false;
    tmpnam(inflatedFilename);
    strcat(inflatedFilename, ".vgm");
    gzFile gz = gzopen(infile, "rb");
    if (!gz)
    {
        fprintf(stderr, "Can't open gzip file : %s\n", infile);
        return false;
    }
    else
    {
        FILE *outFile = fopen(inflatedFilename, "wb");
        if (outFile)
        {
            const int IO_SIZE = 512;
            unsigned char ioBuffer[IO_SIZE];
            int bytesRead = -1;
            int totalBytesRead = 0;
            while ((bytesRead = gzread(gz, ioBuffer, IO_SIZE)) != -1)
            {
                if (fwrite(ioBuffer, 1, bytesRead, outFile) != bytesRead)
                {
                    fprintf(stderr, "Failed to write %ld bytes\n", bytesRead);
                    break;
                }
                totalBytesRead += bytesRead;
                if (bytesRead < IO_SIZE) {
                    success = true;
                    break;
                }
            }
            fclose(outFile);
            if (verbose)
                fprintf(stderr, "Gunzipped %ld kB\n", totalBytesRead / 1024);
            if (success) {
              strcpy(infile, inflatedFilename);
            }
        }
    }
    return success;
}


// Deflate the GYM infile to a temporary file, replace the 
// infile with the tmp file path.
bool inflateGYM(char *infile, char *inflatedFilename, bool verbose)
{
    bool result = false;
    const int HEADER_SIZE = 428;

    FILE *input = fopen(infile, "rb");
    fseek(input, 0, SEEK_END);
    const int size = ftell(input);
    rewind(input);

    unsigned char *data = (unsigned char *)malloc(size + 1);
    fread(data, 1, size, input);
    fclose(input);
    const int inflatedSize = data[427] << 24 | data[426] << 16 | data[425] << 8 | data[424];
    if (inflatedSize == 0)
    {   
        // Not compressed
        return result;
    }

    // Space for inflated data, copy header, then clear the compression indicator
    unsigned char *inflated = (unsigned char *)malloc(inflatedSize + HEADER_SIZE + 1);
    memcpy(inflated, data, HEADER_SIZE);
    inflated[424] = 0;
    inflated[425] = 0;
    inflated[426] = 0;
    inflated[427] = 0;

    // Inflate it
    z_stream zStream;
    int ret;
    zStream.zalloc = Z_NULL;
    zStream.zfree = Z_NULL;
    zStream.opaque = Z_NULL;
    zStream.avail_in = size - HEADER_SIZE;
    zStream.next_in = (z_const Bytef *)&data[HEADER_SIZE];
    ret = inflateInit2(&zStream, 0x20 | 15);
    if (ret != Z_OK)
    {
        fprintf(stderr, "Inflate init %ld\n", ret);
        free(inflated);
        return result;
    }
    zStream.avail_out = inflatedSize;
    zStream.next_out = (Bytef *)&inflated[HEADER_SIZE];
    ret = inflate(&zStream, Z_SYNC_FLUSH);
    if (!(ret == Z_OK || ret == Z_STREAM_END))
    {
        fprintf(stderr, "Inflate error %ld\n", ret);
        free(inflated);
        return result;
    }
    inflateEnd(&zStream);

    // Write into a temp file
    tmpnam(inflatedFilename);
    strcat(inflatedFilename, ".sym");
    FILE *output = fopen(inflatedFilename, "wb");
    if (output)
    {
        const int outSize = inflatedSize + HEADER_SIZE;
        if (fwrite(inflated, 1, outSize, output) == outSize)
        {
            result = true;
            strcpy(infile, inflatedFilename);
        }
        fclose(output);
    }
    free(inflated);
    free(data);
    return result;
}

int main(int argc, char** argv)
{
    // Command-line arguments
    int vflag = 0;
    //char *infile = NULL;
    char infile[128];
    int iflag = 0;
    int c = 0;
    int t_sec = 30; // default: 30 seconds of wave
    int tflag = 0;
    int sel_voice = 0;
    int sflag = 0;
    int oflag = 0;
    int verbose = 0;
    char *outfile = NULL;
    int trflag = 0;
    int tr_sel = 0;
    int freq_sel = 0;
    int freqflag = 0;
    int output8bit = 0;

    if (argc < 2) 
    {
        usage();
    }

    while ((c = getopt( argc, argv, "vb8t:i:s:o:r:f:")) != -1 )
        switch (c)
        {
            case 'v': // all voices
              vflag = 1;
              break;
            case 'i': // input file
              strcpy(infile, optarg);
              iflag = 1;
              break;
            case 't': // change the time
              t_sec = atoi( optarg );
              tflag = 1;
              break;
            case 's': // single voice
              sel_voice = atoi( optarg );
              sflag = 1;
              break;
            case 'o':
              outfile = optarg;
              oflag = 1;
              break;
            case 'b':
              verbose = 1;
              break;
            case 'r':
              trflag = 1;
              tr_sel = atoi( optarg );
              break;
            case 'f':
              freqflag = 1;
              freq_sel = atoi( optarg );
              break;
            case '8':
              output8bit = 1;
              break;
            case '?':
              if (optopt == 'i')
                  fprintf(stderr, "Option -%c requires an argument.\n", optopt);
              else if (isprint (optopt))
                  fprintf(stderr, "Unknown option `-%c'.\n", optopt);
              else
                  fprintf(stderr,
                          "Unknown option character `\\x%x'/\n",
                          optopt);
              return 1;
            default:
              abort();
        }
	
    unsigned int sample_rate = 44100; /* number of samples per second */
    if (freqflag)
        sample_rate = freq_sel;
    if (verbose)
        fprintf(stderr, "Sample rate : %ld\n", sample_rate);

	int track = ( trflag == 1 ) ? tr_sel : 0; /* index of track to play (0 = first) */
	
    if (!iflag)
        sprintf(infile, "test.nsf");
    if (verbose)
        fprintf(stderr, "Input file : %s\n", infile);

    /**
     * Convert vgz into vgm if needed
     */
    char inflatedFilename[64];
    bool removeInflateTmpFile = false;
    char *fileExt = strrchr(infile, '.');
    if (fileExt)
    {
        for (char *p = fileExt; *p; ++p)
            *p = tolower(*p);
        // Try ungzipping vgz fils
        if (strcmp(fileExt, ".vgz") == 0)
        {
            if (unGzip(infile, inflatedFilename, verbose))
            {
                removeInflateTmpFile = true;
            }
        }
        // Try deflating GYM files
        else if (strcmp(fileExt, ".gym") == 0)
        {
            if (inflateGYM(infile, inflatedFilename, verbose))
            {
                removeInflateTmpFile = true;
            }
        }
    }

    Music_Emu *emu;
    /* Open music file in new emulator */
	handle_error( gme_open_file( infile, &emu, sample_rate ) );
	
    /* Get num voices */
    int num_voices = ( vflag == 1 ) ? gme_voice_count( emu ) : 1;
    
    /* Decide on how long to play.
     * If time specified, play the whole track.
     * If no track length specified, default to 30 seconds.
     */
    if ( !tflag )
    {
        gme_info_t* tinfo;
        handle_error( gme_track_info( emu, &tinfo, track ) );
        int tlen = tinfo->play_length;
        if ( tinfo->length != -1 ) // if the length is specified
            t_sec = tinfo->length / 1000;
        else // if the length is not specified
            t_sec = (tlen == 150000) ? T_SEC_DEFAULT : (tlen / 1000);
        if (verbose)
            fprintf(stderr, "Track length : %d s\n", t_sec);
    }
    // process each voice if -v is enabled
    for (int vi = 0; vi < num_voices; vi++)
    {
        
        // unsilence tracks. 1 means mute
        #define ALL_VOICES -1
        if ( sflag == 1 ) 
        {
            gme_mute_voices( emu, ALL_VOICES );
            gme_mute_voice( emu, sel_voice, 0 ); 
            if (verbose)
                fprintf(stderr, "Unmuted voice %d\n", sel_voice);
        }
        else if ( vflag == 1 ) 
        {
            // unmute only the currently processed voice
            gme_mute_voices( emu, ALL_VOICES );
            gme_mute_voice( emu, vi, 0 );
            if (verbose)
                fprintf(stderr, "Unmuted voice %d\n", vi);
        }

        /* Start track */
        handle_error( gme_start_track( emu, track ) );
        
        char fname[80];
        if ( vflag == 1 )
            sprintf( fname, "Voice%d.wav", vi );
        else if ( oflag == 1 )
            strcpy( fname, outfile );
        else // no output set
        {
            if ( sflag == 1)
                sprintf( fname, "Voice%d.wav", sel_voice);
            else // default    
                sprintf( fname, "out.wav" );
        }

        if (verbose)
            fprintf(stderr, "Output file : %s\n", fname);
        
        /* Begin writing to wave file */
        FILE * curfile = wave_open( sample_rate, fname );
        wave_enable_stereo();
        if (output8bit) wave_set_8bit();

        /* Record t_sec (default 10) seconds of track */
        int done = 0;
        while ( gme_tell( emu ) < t_sec * 1000L )
        {
            /* Sample buffer */
            #define BUF_SIZE 1024 /* can be any multiple of 2 */
            short buf [BUF_SIZE];
            
            /* Fill sample buffer */
            handle_error( gme_play( emu, BUF_SIZE, buf ) );
            
            /* Write samples to wave file */
            wave_write( buf, BUF_SIZE );
        }

        /* Done writing voice to wave, write header */
        wave_write_header();
        /* Output temp file to stdout if -o - */ 
        if (strcmp (fname, "-" ) == 0)
        {
            errno = 0;
            rewind(curfile);
            if (!errno)
            {
                size_t buflen;
                char buf[2048];
                while ((buflen = fread(buf, 1, sizeof buf, curfile)) > 0)
                    fwrite(buf, 1, buflen, stdout);
            }
        }
        /* now close the file */
        wave_close();
    }

	/* Cleanup */
	gme_delete( emu );
	
    if (removeInflateTmpFile) {
        remove(inflatedFilename);
    }

	return 0;
}

void handle_error( const char* str )
{
	if ( str )
	{
		fprintf(stderr, "Error: %s\n", str ); getchar();
		exit( EXIT_FAILURE );
	}
}

void usage(void)
{
    fprintf(stderr, "usage: vgm2wav -i [file] -o [file] [-v] [-r track_num] [-f freq] [-b] [-8] [-t secs]\n" );
    fprintf(stderr, "output supports '-' as filename for stdout\n");
    fprintf(stderr, "-i: input file (AY,GBS,GYM,HES,KSS,NSF/NSFE,SAP,SPC,VGM,VGZ)\n");
    fprintf(stderr, "-o: output file (WAV)\n");
    fprintf(stderr, "-v: all voices into separate WAVs\n");
    fprintf(stderr, "-r: index of track to play (0 is first)\n");
    fprintf(stderr, "-f: output sample frequency (default 44100)\n");
    fprintf(stderr, "-b: verbose output\n");
    fprintf(stderr, "-8: output 8-bit WAV instead of 16-bit WAV\n");
    fprintf(stderr, "-t: playtime in secs (default 30s or whatever is defined in the file)\n");
    exit( EXIT_FAILURE );
}
