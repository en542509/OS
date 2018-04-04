#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "xis.h"
#include "xcpu.h"
#include "xmem.h"

/* Use
 *   xcpu_print( c );
 * to print cpu state, where c is of type xcpu *
 * See xcpu.h for full prototype of xcpu_print()
 */
extern unsigned char *memory;

extern int xcpu_execute( xcpu *c ) {

  /* Your code here */
  if ( c == NULL || memory == NULL )
    return 0;

  unsigned char instruction[2];
  // fetch
  xmem_load( c->pc, instruction );
  unsigned char byte1 = instruction[0];
  unsigned char byte2 = instruction[1];
  unsigned short temp = 0;

  // increment PC
  c->pc += 2;

  // decode and excute
  switch ( byte1 ) {
    /* OPS  OPNUM    Operations with no operands */
    case I_RET: /*  00  000001 */
      c->pc = memory[ c->regs[15] ];
          c->regs[15] += 2;
          break;

    case I_CLD: /*  00  000010 */
      c->state &= 0xFFFD;
          break;

    case I_STD: /*  00  000011 */
      c->state |= 0x0002;
          break;

          /* OPS  I OPNUM   Operations with one register operand */
    case I_NEG: /*  01  0 00001  */
      temp = c->regs[ XIS_REG1( byte2 ) ];
          c->regs[ XIS_REG1( byte2 ) ] = temp;
          break;

    case I_NOT: /*  01  0 00010  */
      temp = c->regs[ XIS_REG1( byte2 ) ];
          c->regs[ XIS_REG1( byte2 ) ] = !temp;
          break;

    case I_INC: /*  01  0 01000  */
      c->regs[ XIS_REG1( byte2 ) ] += 1;
          break;

    case I_DEC: /*  01  0 01001  */
      c->regs[ XIS_REG1( byte2 ) ] -= 1;
          break;

    case I_PUSH: /*  01  0 00011  */
      c->regs[15] -= 2;
          memory[ c->regs[15] ] = c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_POP: /*  01  0 00100  */
      c->regs[ XIS_REG1( byte2 ) ] = memory[ c->regs[15] ];
          c->regs[15] += 2;
          break;

    case I_JMPR: /*  01  0 00101  */
      c->pc = c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_CALLR: /*  01  0 00110  */
      c->regs[15] -= 2;
          memory[ c->regs[15] ] = c->pc;
          c->pc = c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_OUT: /*  01  0 00111  */
      printf( "%c", c->regs[ XIS_REG1( byte2 ) ] );
          break;

          /* OPS  I OPNUM   Operations with one immediate operand */
    case I_BR: /*  01  1 00001  */
      if ( (c->state & 0x0001) == 0x001 )
        c->pc += byte2;
          break;

    case I_JR: /*  01  1 00010  */
      c->pc += byte2;
          break;

          /* OPS  OPNUM   Operations with two register operands */
    case I_ADD: /*  10  000001  */
      c->regs[ XIS_REG2( byte2 ) ] += c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_SUB: /*  10  000010  */
      c->regs[ XIS_REG2( byte2 ) ] -= c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_MUL: /*  10  000011  */
      c->regs[ XIS_REG2( byte2 ) ] *= c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_DIV: /*  10  000100  */
      c->regs[ XIS_REG2( byte2 ) ] /= c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_AND: /*  10  000101  */
      c->regs[ XIS_REG2( byte2 ) ] &= c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_OR: /*  10  000110  */
      c->regs[ XIS_REG2( byte2 ) ] |= c->regs[ XIS_REG1( byte2 ) ];
          break;
    case I_XOR: /*  10  000111  */
      c->regs[ XIS_REG2( byte2 ) ] ^= c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_SHR: /*  10  001000  */
      c->regs[ XIS_REG2( byte2 ) ] >>= c->regs[ XIS_REG1(byte2) ];
          break;

    case I_SHL: /*  10  001001  */
      c->regs[ XIS_REG2(byte2) ] <<= c->regs[ XIS_REG1(byte2) ];
          break;

    case I_TEST: /*  10  001010  */
      if ( ( c->regs[ XIS_REG1(byte2) ] & c->regs[ XIS_REG2( byte2 ) ] ) !=\
 0)
        c->state |= 0x0001;
      else
        c->state &= 0xFFFE;
          break;

    case I_CMP: /*  10  001011  */
      if ( c->regs[ XIS_REG1( byte2 ) ] < c->regs[ XIS_REG2( byte2 ) ] )
        c->state |= 0x0001;
      else
        c->state &= 0xFFFE;
          break;
    case I_EQU: /*  10  001100  */
      if ( c->regs[ XIS_REG1( byte2 ) ] == c->regs[ XIS_REG2( byte2 ) ] )
        c->state |= 0x0001;
      else
        c->state &= 0xFFFE;
          break;

    case I_MOV: /*  10  001101  */
      c->regs[ XIS_REG2( byte2 ) ] = c->regs[ XIS_REG1( byte2 ) ];
          break;

    case I_LOAD: /*  10  001110  */
      c->regs[ XIS_REG2( byte2 ) ] = memory[ c->regs[ XIS_REG1( byte2 ) ] ]\
;
          break;

    case I_STOR: /*  10  001111  */
      memory[ c->regs[ XIS_REG2( byte2 ) ] ] = c->regs[ XIS_REG1( byte2 ) ]\
;
          break;

    case I_LOADB: /*  10  010000  */
      c->regs[ XIS_REG2( byte2 ) ] = memory[ c->regs[ XIS_REG1( byte2 ) ] ]\
;
          break;
    case I_STORB: /*  10  010001  */
      memory[ c->regs[ XIS_REG2( byte2 ) ] ] = c->regs[ XIS_REG1( byte2 ) ]\
;
          break;

          /* OPS  R OPNUM   X Operations with one immediate operand */
    case I_JMP: /*  11  0 00001 */
      xmem_load( c->pc, instruction );
          c->pc = instruction[0] << ( 8 + instruction[1] );
          break;

    case I_CALL: /*  11  0 00010 */
      c->regs[15] -= 2;
          memory[c->regs[15]] = c->pc;
          xmem_load( c->pc, instruction );
          c->pc = instruction[0] << ( 8 + instruction[1] );
          break;
          /* OPS  R OPNUM   X Ops with 1 imm. and 1 reg. operand */
    case I_LOADI: /*  11  1 00001 */
      c->regs[ XIS_REG1( byte2 ) ] = memory[ c->pc ];
          c->pc += 2;
          break;

    default:
      return 0;

  }

  if ( c->state > 0)
    xcpu_print(c);

  return 1; /* replace this as needed */
}


/* Not needed for assignment 1 */
int xcpu_exception( xcpu *c, unsigned int ex ) {
  return 0;
}