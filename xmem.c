#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xis.h"
#include "xmem.h"
#include "block.h"
#include "pthread.h"

static unsigned char *memory;
static int phys_size;
pthread_mutex_t mem_lock = PTHREAD_MUTEX_INITIALIZER;

/* title: init memory with a specific size
 * param: size of physical memory (between 0 and 65536)
 * function: iniializes physical memory of specified size
 * returns: 1 if successful, 0 if not
 */
extern int xmem_init( int size ) {
  if( size <= XIS_MEM_SIZE ) {
    phys_size = size;
    memory = calloc( 1, size );
    return memory != NULL;
  }
  return 0;
}

// In this function, we will use paging and block_store to imple virtual memory.
// 1. unsigned short addr : one level paging 
//            15        8 7         0 bit
//            +----------+----------+
//            |page table|  shift   |
//            +----------+----------+
// 2. cause block_store/load must copy the whole BLOCK_SIZE(256) byte, 
//    and we cannot malloc 256 bytes space, so we will use the top area 
//    of "memory" as buffer to store BLOCK_SIZE virtual memory


// we will not check addr cause it will fit the size of virtual memory (65536)
extern void xmem_store( unsigned char data[2], unsigned short addr ) {
  unsigned short sf = addr & 0xff;           // get low 8 bit;
  unsigned short pt = (addr >> 8) & 0xff;    // get high 8 bit;
  unsigned char* cpm = memory + phys_size - 256; 

  pthread_mutex_lock(&mem_lock);
  block_load(pt, cpm);  
  memcpy(cpm + sf, data, 2);
  block_store(cpm, pt);
  pthread_mutex_unlock(&mem_lock);
}

// 
extern void xmem_load( unsigned short addr, unsigned char data[2] ) {
  unsigned short sf = addr & 0xff;           // get low 8 bit;
  unsigned short pt = (addr >> 8) & 0xff;    // get high 8 bit;
  unsigned char* cpm = memory + phys_size - 512;

  pthread_mutex_lock(&mem_lock);
  if(block_load(pt, cpm)) { // if we find data
    memcpy(data, cpm + sf, 2);
  }
  else { // we don't find data, error
    printf( "Memory load error at virtual addr: %d, page table: %d, shift: %d\n", addr, pt, sf );
    pthread_mutex_unlock(&mem_lock);
    exit(1);
  }
  
  pthread_mutex_unlock(&mem_lock);
}



