#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "xis.h"
#include "xcpu.h"
#include "xmem.h"

#define TICK_ARG 1
#define IMAGE_ARG 2

int main( int argc, char **argv ) {

  FILE *fp;
  struct stat fs;
  xcpu cs;
  unsigned char data[2];
  int ticks;
  unsigned int i;

  if( ( argc < 3 ) || ( sscanf( argv[TICK_ARG], "%d", &ticks ) != 1 ) || 
      ( ticks < 0 ) ) {
    printf( "usage: xsim <ticks> <obj file>\n" );
    printf( "      <ticks> is number instructions to execute (0 = forever)\n" );
    printf( "      <image file> xis object file created by or xasxld\n" );
    return 1;
  } 

  if( !xmem_init( XIS_MEM_SIZE ) ) {
    printf( "error: memory allocation (%d) failed\n", XIS_MEM_SIZE );
    exit( 1 );
  } else if( stat( argv[IMAGE_ARG], &fs ) ) {
    printf( "error: could not stat image file %s\n", argv[IMAGE_ARG] );
    return 1;
  } else if( fs.st_size > XIS_MEM_SIZE ) {
    printf( "Not enough memory to run all the programs." );
    return 1;
  }

  fp = fopen( argv[IMAGE_ARG], "rb" );
  if( !fp ) {
    printf( "error: could not open image file %s\n", argv[IMAGE_ARG] );
    return 1;
  } 
 
  for( i = 0; i < fs.st_size; i += 2 ) {
    if( fread( data, 1, 2, fp ) < 1 ) {
      printf( "error: could not read file %s\n", argv[IMAGE_ARG] );
      return 1;
    }
    xmem_store( data, i );
  }
  fclose( fp );

  memset( &cs, 0, sizeof( xcpu ) );

  for( i = 0; ( ticks < 1 ) || ( i < ticks ); i++ ) {
    if( !xcpu_execute( &cs ) ) {
      break;
    }
  }

  return 0;
}


