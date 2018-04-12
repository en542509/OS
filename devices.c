#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "xdev.h"
#include "devices.h"

#define DISP_PORT 1
#define KBD_PORT 2
#define RAND_PORT 3

static void do_or_die( int success, char *errmsg );

/* title: display device
 * param: void *arg (ignored)
 * function: invoked on a separate thread and peforms the following operations:
 *           - associates device with port 1 
 *           - enters an infinite loop
 *           - the loop 
 *             - gets a charatcer (lower half of a word) from the port
 *             - outputs the character to stdout
 * returns: void *, never returns.
 */
extern void *device_display( void *arg ) {
  unsigned short data;

  do_or_die( xdev_associate_port( DISP_PORT ), 
            "xdev_associate_port( DISP_PORT ) failed" );

  while( 1 ) {
    do_or_die( xdev_dev_get( DISP_PORT, &data ), 
              "xdev_dev_get( DISP_PORT, &data ) failed" );
    putchar( data );
    fflush( stdout );
  }
  return NULL;
}


/* title: keyboard device
 * param: void *arg (ignored)
 * function: invoked on a separate thread and peforms the following operations:
 *           - associates device with port 2 
 *           - enters an infinite loop
 *           - the loop 
 *             - gets a character from stdin
 *             - puts the character (lower half of a word) to the port
 * returns: void *, never returns.
 */
extern void *device_keyboard( void *arg ) {
  unsigned short data;

  do_or_die( xdev_associate_port( KBD_PORT ), 
            "xdev_associate_port( KBD_PORT ) failed" );

  while( 1 ) {
    data = (unsigned short) getchar();
    do_or_die( xdev_dev_put( data, KBD_PORT ), 
              "xdev_dev_put( data, KBD_PORT ) failed" );
  }
  return NULL;
}


/* title: random device
 * param: void *arg (ignored)
 * function: invoked on a separate thread and peforms the following operations:
 *           - associates device with port 3 
 *           - enters an infinite loop
 *           - the loop 
 *             - generates a random word using random()
 *             - puts the word to the port
 * returns: void *, never returns.
 */
extern void *device_random( void *arg ) {
  unsigned short data;

  do_or_die( xdev_associate_port( RAND_PORT ), 
            "xdev_associate_port( RAND_PORT ) failed" );

  while( 1 ) {
    data = (unsigned short) random();
    do_or_die( xdev_dev_put( data, RAND_PORT ), 
              "xdev_dev_put( data, RAND_PORT ) failed" );
  }
  return NULL;
}


static void do_or_die( int success, char *errmsg ) {
  if( !success ) {
    printf( "internal error: %s\n", errmsg );
    abort();
  }
}
