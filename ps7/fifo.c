/* Mark Bryk OS PS7 FIFO.C */
#include "fifo.h"
#include <string.h>
#include <sys/mman.h>

void fifo_init(struct fifo *f){
	memset(f->buf, '\0', BSIZE);
	f->next_read = 0; f->next_write = 0;

	struct sem *mutex = mmap(NULL, sizeof(struct sem), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
	struct sem *full = mmap(NULL, sizeof(struct sem), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
	struct sem *empty = mmap(NULL, sizeof(struct sem), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
	f->mutex = mutex; f->full = full; f->empty = empty;
	sem_init(f->mutex,1); sem_init(f->full,0); sem_init(f->empty,BSIZE);
}

void fifo_wr(struct fifo *f, unsigned long d){
	sem_wait(f->empty);
	sem_wait(f->mutex);
	
	f->buf[f->next_write++] = d;
	f->next_write %= BSIZE;

	sem_inc(f->mutex);
	sem_inc(f->full);
}

unsigned long fifo_rd(struct fifo *f){
	sem_wait(f->full);
	sem_wait(f->mutex);	

	unsigned long d = f->buf[f->next_read++];
	f->next_read %= BSIZE;

	sem_inc(f->mutex);
	sem_inc(f->empty);
	return d;
}
