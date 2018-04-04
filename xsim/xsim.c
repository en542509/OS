#include <stdio.h>
#include <stdlib.h>
#include "xis.h"
#include "xcpu.h"
#include <string.h>
#include "xmem.h"

extern unsigned char *memory;

int main( int argc, char **argv )
{
    if ( argc < 3 )
    {
        printf( "Invalid arguments!\n" );
        return 0;
    }

    int total = 0;

    if ( sscanf( argv[1], "%d", &total ) <= 0 || total < 0 )
    {
        printf( "Negative maximum cycles!\n" );
        return 0;
    }

    char *filename = argv[2];
    FILE *fin = fopen( filename, "rb" );

    if ( !fin )
    {
        printf( "File open failed!\n" );
        return 0;
    }

    xcpu c;
    memset( &c, 0, sizeof( xcpu ) );
    xmem_init( 65536 );
    fseek( fin, 0, SEEK_END );
    long size = ftell( fin );

    if ( size <= 0 )
    {
        fclose( fin );
        printf( "Empty file!\n" );
        return 0;
    }

    rewind( fin );
    fread( memory, 1, size, fin );
    fclose( fin );

    if ( total > 0 )
    {
        int i = 0;
        while ( i < total )
        {
            unsigned char instruction[2];
            xmem_load( c.pc, instruction );

            if ( instruction[0] == 0 )
                break;

            xcpu_execute( &c );
            i++;
        }
    }
    else
    {
        while ( 1 )
            xcpu_execute( &c );
    }

    free(memory);

    return 0;
}