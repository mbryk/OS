/* Mark Bryk OS PS7 MAIN.C */
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include "fifo.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N_PROC 64
sigset_t mask;
static void handler(int sn){ }
int main(int argc, char **argv){
	sigfillset(&mask);
	sigdelset(&mask,SIGUSR1);
	signal(SIGUSR1, handler);
	struct fifo *f = mmap(NULL, sizeof(struct fifo), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
	fifo_init(f);
	int i,p; int j = 0;
	int readers = 3; int writers = 3;
	int pn2pid[readers+writers];
	unsigned long d;
	for(i=0; i<readers; i++){ /* Readers */
		p=fork();
		if(p==-1){perror("fork"); return -1;}
		else if(!p){
			for(j=0;j<2;j++){
				p = getpid();
				d = fifo_rd(f);
				fprintf(stderr, "%d read %lu\n",p,d);
			}
			return 0;
		}
		pn2pid[i]=p;
	}
	fprintf(stderr, "Readers:\n");
	for(j=0; j<readers;j++){
		fprintf(stderr,"procnum[%d]=process %d\n",j,pn2pid[j]);
	}
	for(i=0; i<writers; i++){
		p=fork();
		if(p==-1){perror("Fork"); return -1;}
		else if(!p){
			for(j=0;j<2;j++){
				p = getpid();
				d = i*10+j+5;
				fifo_wr(f,d);
				fprintf(stderr, "%d wrote %lu\n",p,d);
			}
			return 0;
		}
		pn2pid[readers+i] = p;
	}
	fprintf(stderr, "Writers:\n");
	for(j=0;j<writers;j++){
		fprintf(stderr, "procnum[%d]=process %d\n", j, pn2pid[readers+j]);
	}

	int stat;
	for(i=0;i<readers+writers;i++)
		wait(&stat);				
	return 0;
}
