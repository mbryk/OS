/* Mark Bryk OS PS7 FIFO.H */
#ifndef FIFO.H
#define FIFO_H

#include "sem.h"

#define BSIZE 4096

volatile struct fifo{
	unsigned long buf[BSIZE];
	int item_count;
	int next_read;
	int next_write;
	struct sem *s;
}
void fifo_init(struct fifo*);
void fifo_wr(struct fifo *, unsigned long);
unsigned long fifo_rd(struct fifo*);

 
#endif
