#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xis.h"
#include "xcpu.h"
#include "xmem.h"

#define MEM_IDX(x) ((x) % XIS_MEM_SIZE)

static void push( xcpu *c, unsigned short v ) {
  unsigned char data[2] = { v >> 8, v };
  c->regs[X_STACK_REG] -= 2;
   
  xmem_store( data, MEM_IDX(c->regs[X_STACK_REG]) );
}

static unsigned short pop( xcpu *c ) {
  unsigned char data[2];
  xmem_load( MEM_IDX(c->regs[X_STACK_REG]), data );
  c->regs[X_STACK_REG] += 2;
  return data[0] << 8 | data[1];
}

extern int xcpu_execute( xcpu *c ) {
  unsigned char op;
  char op2;
  int r1;
  int r2;
  unsigned short val;
  unsigned char data[2];

  assert( c );

  xmem_load( MEM_IDX(c->pc), data );
  op = data[0];
  op2 = data[1];
  c->pc = MEM_IDX(c->pc + 2);
  r1 = ( op2 >> 4 ) & 0xf;
  r2 = op2 & 0xf;
 
  if( XIS_IS_EXT_OP( op ) ) { 
    xmem_load( MEM_IDX(c->pc), data );
    val = data[0] << 8 | data[1];
    c->pc = MEM_IDX(c->pc + 2);
  }

  switch( op ) {
  case I_PUSH:
    push( c, c->regs[r1] );
    break;
  case I_POP:
    c->regs[r1] = pop( c );
    break;
  case I_MOV:
    c->regs[r2] = c->regs[r1];
    break; 
  case I_LOAD:
    xmem_load( MEM_IDX(c->regs[r1]), data );
    c->regs[r2] = data[0] << 8 | data[1];
    break; 
  case I_STOR:
    data[0] = c->regs[r1] >> 8;
    data[1] = c->regs[r1];
    xmem_store( data, MEM_IDX(c->regs[r2]) );
    break; 
  case I_LOADB:
    xmem_load( MEM_IDX(c->regs[r1]), data );
    c->regs[r2] = data[0];
    break; 
  case I_STORB:
    xmem_load( MEM_IDX(c->regs[r2]), data );
    data[0] = (char) c->regs[r1];
    xmem_store( data, MEM_IDX(c->regs[r2]) );
    break; 
  case I_JMPR:
    val = c->regs[r1];
  case I_JMP:
    c->pc = val;
    break; 
  case I_CALLR:
    val = c->regs[r1];
  case I_CALL:
    push( c, c->pc );
    c->pc = val;
    break; 
  case I_RET:
    c->pc = pop( c );
    break; 
  case I_LOADI:
    c->regs[r1] = val;
    break; 
  case I_ADD:
    c->regs[r2] += c->regs[r1];
    break; 
  case I_SUB:
    c->regs[r2] -= c->regs[r1];
    break; 
  case I_MUL:
    c->regs[r2] *= c->regs[r1];
    break; 
  case I_DIV:
    c->regs[r2] /= c->regs[r1];
    break; 
  case I_NEG:
    c->regs[r1] = -c->regs[r1];
    break; 
  case I_AND:
    c->regs[r2] &= c->regs[r1];
    break; 
  case I_OR:
    c->regs[r2] |= c->regs[r1];
    break; 
  case I_XOR:
    c->regs[r2] ^= c->regs[r1];
    break; 
  case I_NOT:
    c->regs[r1] = !c->regs[r1];
    break; 
  case I_INC:
    c->regs[r1]++;
    break; 
  case I_DEC:
    c->regs[r1]--;
    break; 
  case I_SHL:
    c->regs[r2] <<= c->regs[r1];
    break; 
  case I_SHR:
    c->regs[r2] >>= c->regs[r1];
    break; 
  case I_TEST:
    if( c->regs[r2] & c->regs[r1] ) {
      c->state |= X_STATE_COND_FLAG;
    } else {
      c->state &= ~X_STATE_COND_FLAG;
    }
    break; 
  case I_CMP:
    if( c->regs[r1] < c->regs[r2] ) {
      c->state |= X_STATE_COND_FLAG;
    } else {
      c->state &= ~X_STATE_COND_FLAG;
    }
    break; 
  case I_EQU:
    if( c->regs[r1] == c->regs[r2] ) {
      c->state |= X_STATE_COND_FLAG;
    } else {
      c->state &= ~X_STATE_COND_FLAG;
    }
    break; 
  case I_BR:
    if( c->state & X_STATE_COND_FLAG ) {
      c->pc += op2 - 2;
    }
    break; 
  case I_JR: 
    c->pc += op2 - 2;
    break; 
  case I_CLD:
    c->state &= ~X_STATE_DEBUG_ON;
    break; 
  case I_STD:
    c->state |= X_STATE_DEBUG_ON;
    break; 
  case I_OUT:
    putchar( c->regs[r1] );
    break; 
  case I_BAD:
  default:
    return 0;
  }

  if( c->state & X_STATE_DEBUG_ON ) {
    xcpu_print( c );
  }

  return 1;
}



int xcpu_exception( xcpu *c, unsigned int ex ) {
  return 0;
}
