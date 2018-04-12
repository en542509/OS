#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "xdev.h"

enum {
  INIT,
  UNBLOCKED,
  OUT_BLOCKED,
  IN_BLOCKED,
  BLOCKED,
};

typedef struct req_rec reqrec;
typedef struct port_rec portrec;
typedef struct req_que req_q;

struct req_que {
  reqrec *head;
  reqrec *tail;
};

struct req_rec {
  reqrec    		*next;
  int			state;
  pthread_cond_t	cond_var;
  unsigned short	data;
  unsigned short	*loc;
};

struct port_rec {
  req_q			in_q;
  req_q			out_q;
  int			state;
  unsigned short	data;
  unsigned short	*loc;
  pthread_cond_t	cond_var;
};

static portrec ports[65536];
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static reqrec *freelist;

static int outp( unsigned short data, unsigned short port, int sync );
static int inp( unsigned short port, unsigned short *data, int sync );
static void do_or_die( int success, char *errmsg );
static reqrec *q_rec( unsigned short data, unsigned short *loc );
static void q_free( reqrec *r );
static int q_empty( req_q *q );
static reqrec *q_dequeue( req_q *q );
static void q_enqueue( req_q *q, reqrec *r );



int xdev_associate_port( unsigned short port ) {
  portrec *p = &ports[port];

  do_or_die( !pthread_mutex_lock( &mutex ), "mutex lock failed" );
 
  if( !p->state ) {
    p->state = UNBLOCKED;
    pthread_cond_init( &p->cond_var, NULL );
  } else {
    p = NULL;
  }

  do_or_die( !pthread_mutex_unlock( &mutex ), "mutex unlock failed" );

  return p != NULL;
}
  

int xdev_dev_put( unsigned short data, unsigned short port ) {
  portrec *p = &ports[port];
  reqrec *r;

  do_or_die( !pthread_mutex_lock( &mutex ), "mutex lock failed" );
 
  if( p->state ) {
    if( q_empty( &p->in_q ) ) {
      p->data = data;
      for( p->state = IN_BLOCKED; p->state == IN_BLOCKED; ) {
        do_or_die( !pthread_cond_wait( &p->cond_var, &mutex ), 
                   "cond_wait failed" );
      }
    } else {
      r = q_dequeue( &p->in_q );
      *r->loc = data;
      r->state = UNBLOCKED;
      do_or_die( !pthread_cond_signal( &r->cond_var ), "cond_signal failed" );
    }
  } else {
    p = NULL;
  }

  do_or_die( !pthread_mutex_unlock( &mutex ), "mutex unlock failed" );

  return p != NULL;
}


int xdev_dev_get( unsigned short port, unsigned short *data ) {
  portrec *p = &ports[port];
  reqrec *r;

  do_or_die( !pthread_mutex_lock( &mutex ), "mutex lock failed" );
 
  if( p->state ) {
    if( q_empty( &p->out_q ) ) {
      p->loc = data;
      for( p->state = OUT_BLOCKED; p->state == OUT_BLOCKED; ) {
        do_or_die( !pthread_cond_wait( &p->cond_var, &mutex ), 
                   "cond_wait failed" );
      }
    } else {
      r = q_dequeue( &p->out_q );
      *data = r->data;
      if( r->state == BLOCKED ) {
        r->state = UNBLOCKED;
        do_or_die( !pthread_cond_signal( &r->cond_var ), "cond_signal failed" );
      } else {
        q_free( r );
      }
    }
  } else {
    p = NULL;
  }

  do_or_die( !pthread_mutex_unlock( &mutex ), "mutex unlock failed" );

  return p != NULL;
}


int xdev_outp_sync( unsigned short data, unsigned short port ) {
  return outp( data, port, 1 );
}


int xdev_outp_async( unsigned short data, unsigned short port ) {
  return outp( data, port, 0 );
}


int xdev_inp_sync( unsigned short port, unsigned short *data ) {
  return inp( port, data, 1 );
}


int xdev_inp_poll( unsigned short port, unsigned short *data ) {
  return inp( port, data, 0 );
}


static int outp( unsigned short data, unsigned short port, int sync ) {
  portrec *p = &ports[port];
  reqrec *r;

  do_or_die( !pthread_mutex_lock( &mutex ), "mutex lock failed" );
 
  if( p->state ) {
    if( p->state == OUT_BLOCKED ) {
      p->state = UNBLOCKED;
      *p->loc = data;
      do_or_die( !pthread_cond_signal( &p->cond_var ), "cond_signal failed" );
    } else {
      r = q_rec( data, NULL );
      q_enqueue( &p->out_q, r );
      if( sync ) {
        for( r->state = BLOCKED; r->state == BLOCKED; ) {
          do_or_die( !pthread_cond_wait( &r->cond_var, &mutex ), 
                     "cond_wait failed" );
        }
        q_free( r );
      }
    }
  } else {
    p = NULL;
  }

  do_or_die( !pthread_mutex_unlock( &mutex ), "mutex unlock failed" );

  return p != NULL;
}


static int inp( unsigned short port, unsigned short *data, int sync ) {
  portrec *p = &ports[port];
  reqrec *r;

  do_or_die( !pthread_mutex_lock( &mutex ), "mutex lock failed" );
 
  if( p->state ) {
    if( p->state == IN_BLOCKED ) {
      p->state = UNBLOCKED;
      *data = p->data;
      do_or_die( !pthread_cond_signal( &p->cond_var ), "cond_signal failed" );
    } else if( sync ) {
      r = q_rec( 0, data );
      q_enqueue( &p->in_q, r );
      for( r->state = BLOCKED; r->state == BLOCKED; ) {
        do_or_die( !pthread_cond_wait( &r->cond_var, &mutex ), 
                   "cond_wait failed" );
      }
      q_free( r );
    } else {
      p = NULL;
    }
  } else {
    p = NULL;
  }

  do_or_die( !pthread_mutex_unlock( &mutex ), "mutex unlock failed" );

  return p != NULL;
}


static reqrec *q_rec( unsigned short data, unsigned short *loc ) {
  reqrec *r = freelist;

  if( r ) {
    freelist = r->next;
  } else {
    r = malloc( sizeof( reqrec ) );
    do_or_die( r != NULL, "malloc( sizeof( reqrec ) ) failed" );
    memset( r, 0, sizeof( reqrec ) );
  }
  pthread_cond_init( &r->cond_var, NULL );
  r->data = data;
  r->loc = loc;
  return r;
}


static void q_free( reqrec *r ) {
  r->next = freelist;
  freelist = r;
}


static int q_empty( req_q *q ) {
  return q->head == NULL;
}


static reqrec *q_dequeue( req_q *q ) {
  reqrec *r = q->head;

  if( r ) {
    q->head = r->next;
  }

  return r;
}


static void q_enqueue( req_q *q, reqrec *r ) {
  r->next = NULL;
  if( q->head ) {
    q->tail->next = r;
  } else {
    q->head = r;
  }
  q->tail = r;
}


static void do_or_die( int success, char *errmsg ) {
  if( !success ) {
    fprintf( stderr, "internal xdev error: %s\n", errmsg );
    abort();
  }
}
