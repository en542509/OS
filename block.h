#ifndef BLOCKM_H
#define BLOCKM_H

#define BLOCK_SIZE 256

/* This interface simulates a simple block device that allows the user
   to store and load blocks of data (256 bytes) at/with a specific block
   id.  Think of this as a hashtable where the block id is the key and
   and the data is what's being stored.
  */

/* title: store a BLOCK_SIZE block data at specified block id
 * param: pointer to data, block id
 * function: stores BLOCK_SIZE bytes, pointed to by 'data', at block
             'id'.  Id must be in range 0 ... 65535
 * returns: void
 */
extern void block_store( unsigned char *data, unsigned short id ); 


/* title: load a BLOCK_SIZE block of data from specified block id
 * param: block id, pointer to where data should be stored 
 * function: loads a block of BLOCK_SIZE bytes to a memory location 
             pointed to by 'data', idetified by 'id', if a block with
             the same id was stored previously.  Block id must be in logical 
             range (0 ... 65535), and if no block with that id was stored,
             this function returns 0.
 * returns: 0 if block not found, 1 if block is found and returned.
 */
extern int block_load( unsigned short id, unsigned char *data ); 

#endif
