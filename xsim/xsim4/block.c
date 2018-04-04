#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"

static unsigned char *blocks[65536];

static int initialized = 0;
static int stores = 0;
static int loads = 0;

static void results( void ) {
  fprintf( stderr, "Frame loads : %d, Frame stores : %d, Total : %d\n", loads, 
           stores, loads + stores );
}

static void init() {
  initialized = 1;
  atexit( &results ); //call the results_function when the program terminate
}

/* title: store a BLOCK_SIZE block data at specified block id
 * param: pointer to data, block id
 * function: stores BLOCK_SIZE bytes, pointed to by 'data', at block
             'id'.  Id must be in range 0 ... 65525
 * returns: void
 */
extern void block_store( unsigned char *data, unsigned short id ) {
  if( !initialized ) {
    init();
  }
  stores++;

  if( !blocks[id] ) {
    blocks[id] = malloc( BLOCK_SIZE );
  }
  memcpy( blocks[id], data, BLOCK_SIZE );
}


/* title: load a BLOCK_SIZE block of data from specified block id
 * param: block id, pointer to where data should be stored 
 * function: loads a block of BLOCK_SIZE bytes to a memory location 
             pointed to by 'data', idetified by 'id', if a block with
             the same id was stored previously.  Block id must be in logical 
             range (0 ... 65535), and if no block with that id was stored,
             this function returns 0.
 * returns: 0 if block not found, 1 if block is found and returned.
 */
extern int block_load( unsigned short id, unsigned char *data ) {
  if( !initialized ) {
    init();
  }

  if( blocks[id] ) {
    loads++;
    memcpy( data, blocks[id], BLOCK_SIZE );
  }
  return blocks[id] != NULL;
}
