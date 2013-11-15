/* Mark Bryk OS PS7 FIFO.C */
#include "fifo.h"
#include <string.h>
void fifo_init(struct fifo *f){
	memset(f->buf, '\0', BSIZE);
	f->item_count = 0; f->next_read = 0; f->next_write = 0;
	sem_init(f->s,1);
}

void fifo_wr(struct fifo *f, unsigned long d){
	while(1){
		sem_wait(f->s);
		if(f->item_count>=BSIZE)
			sem_inc(f->s);
		else break;
	}
	f->buf[f->next_write++] = d;
	f->next_write %= BSIZE;
	f->item_count++;
	sem_inc(f->s);
}

unsigned long fifo_rd(struct fifo *f){
	while(1){
		sem_wait(f->s);
		if(f->item_count==0)
			sem_inc(f->s);
		else break;
	}
	unsigned long d = f->buf[f->next_read++];
	f->next_read %= BSIZE;
	f->item_count--;
	sem_inc(f->s);
	return d;
}
