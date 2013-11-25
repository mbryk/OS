/* Mark Bryk OS PS7 FIFO.H */
#ifndef FIFO_H
#define FIFO_H

#include "sem.h"

#define BSIZE 4096

struct fifo{
	unsigned long buf[BSIZE];
	int next_read, next_write;
	struct sem mutex,full,empty;
};
void fifo_init(struct fifo*);
void fifo_wr(struct fifo *, unsigned long);
unsigned long fifo_rd(struct fifo*);
#endif
