#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xis.h"
#include "xmem.h"


/* title: init memory with a specific size
 * param: size of physical memory (between 0 and 65536)
 * function: allocates and iniializes physical memory of specified size
 * returns: 1 if successful, 0 if not
 */

unsigned char *memory = NULL;

extern int xmem_init( int size ) {

  /* Your code here */
  memory = (unsigned char *)malloc(size * sizeof(unsigned char));
  if ( memory == NULL )
    return 0;

  return 1;
}


/* title: store a word (2 bytes) of data at specified location in memory
 * param: pointer to data, location in memory
 * function: writes the two bytes at specific address in memory.  Address must
 *           be in logical range (0 ... 65536), and if no virtual mapping
 *           is used must be less than size of physical memory.
 *           this function aborts if an error occurs.
 * returns: void
 */
extern void xmem_store( unsigned char data[2], unsigned short addr ) {

  /* Your code here */
  if ( addr < 0 || addr > 65535 )
  {
    printf( "Invalid address!\n" );
  }
  else
  {
    memory[addr] = data[0];
    memory[addr + 1] = data[1];
  }

}


/* title: load a word (2 bytes) of data from specified location in memory
 * param: location in memory, pointer to where data should be stored
 * function: loads the 2 bytes stored at the specified address into the
             location pointed to by data.  Address must be in logical
             range (0 ... 65536), and if no virtual mapping is used must
             be less than size of physical memory.
 * returns: void
 */
extern void xmem_load( unsigned short addr, unsigned char data[2] ) {

  /* Your code here */
  if ( addr < 0 || addr > 65535 )
  {
    printf( "Invalid address!\n" );
  }
  else
  {
    data[0] = memory[addr];
    data[1] = memory[addr + 1];
  }
}