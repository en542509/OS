#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "xis.h"
#include "xcpu.h"
#include "xmem.h"
#include "devices.h"

#define TICK_ARG 1
#define IMAGE_ARG 2
#define CORES_ARG 3

static int ticks;

static void * run_core( void *arg ) {
  xcpu *cs = (xcpu *) arg;
  unsigned int i;

  for( i = 0; ( ticks < 1 ) || ( i < ticks ); i++ ) {
    if( !xcpu_execute( cs ) ) {
      break;
    }
  }

  return NULL;
}


int main( int argc, char **argv ) {

  FILE *fp;
  struct stat fs;
  xcpu *cs;
  unsigned char data[2];
  int cores;
  unsigned short i;
  pthread_t *tid;
  pthread_t disp_tid, kbd_tid, rand_tid;
  int c ;

  if( ( argc < 4 ) || ( sscanf( argv[TICK_ARG], "%d", &ticks ) != 1 ) || 
      ( ticks < 0 ) || ( sscanf( argv[CORES_ARG], "%d", &cores ) != 1 ) ||
      ( cores < 1 ) ) {
    fprintf( stderr, "usage: xsim <ticks> <obj file> <cores>\n" );
    fprintf( stderr, 
            "      <ticks> is number instructions to execute (0 = forever)\n" );
    fprintf( stderr, 
            "      <image file> is object file created by or xasxld\n" );
    fprintf( stderr, "      <cores> is the # of cores in the system (>= 1)\n" );
    return 1;
  } 

  tid = (pthread_t *)malloc( sizeof( pthread_t ) * cores );
  if( !tid ) {
    fprintf( stderr, "error: memory allocation (%d) failed\n", 
                    (int)sizeof( pthread_t ) * cores );
    exit( 1 );
  }

  if( !xmem_init( XIS_MEM_SIZE ) ) {
    printf( "error: memory allocation (%d) failed\n", XIS_MEM_SIZE );
    exit( 1 );
  } else if( stat( argv[IMAGE_ARG], &fs ) ) {
    fprintf( stderr, "error: could not stat image file %s\n", argv[IMAGE_ARG] );
    return 1;
  } else if( fs.st_size > XIS_MEM_SIZE ) {
    fprintf( stderr, "Not enough memory to run all the programs." );
    return 1;
  }

  fp = fopen( argv[IMAGE_ARG], "rb" );
  if( !fp ) {
    fprintf( stderr, "error: could not open image file %s\n", argv[IMAGE_ARG] );
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

  cs = (xcpu *)malloc( sizeof( xcpu ) * cores );
  if( !cs ) {
    fprintf( stderr, "error: memory allocation (%d) failed\n", 
                   (int)sizeof( xcpu ) * cores );
    exit( 1 );
  }
  memset( cs, 0, sizeof( xcpu ) * cores );

  if( pthread_create( &disp_tid, NULL, device_display, NULL ) ) {
    fprintf( stderr, "error: could not create display device thread\n" );
    exit( 1 );
  } else if( pthread_create( &kbd_tid, NULL, device_keyboard, NULL ) ) {
    fprintf( stderr, "error: could not create keyboard device thread\n" );
    exit( 1 );
  } else if( pthread_create( &rand_tid, NULL, device_random, NULL ) ) {
    fprintf( stderr, "error: could not create random device thread\n" );
    exit( 1 );
  }
  
  c = scanf("%d",&c);
  if (c>=2 && c<= 64){
     xmem_init( c*1024 );
  }

  for( i = 0; i < cores; i++ ) {
    cs[i].id = i;
    cs[i].num = (unsigned short)cores;
    if( pthread_create( &tid[i], NULL, run_core, &cs[i] ) ) {
      fprintf( stderr, "error: could not create thread %d\n", i );
      exit( 1 );
    }
  }

  for( i = 0; i < cores; i++ ) {
    if( pthread_join( tid[i], NULL ) ) {
      fprintf( stderr, "error: could not join thread %d\n", i );
      exit( 1 );
    }
  }

  return 0;
}
