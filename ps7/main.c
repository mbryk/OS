/* Mark Bryk OS PS7 MAIN.C */
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include "fifo.h"
#include <signal.h>
#include <sys/types.h>

#define N_PROC 64
sigset_t mask;
static void handler(int sn){ printf("handled\n");}
int main(int argc, char **argv){
	sigfillset(&mask);
	sigdelset(&mask,SIGUSR1);
	signal(SIGUSR1, handler);
	struct fifo *f = mmap(NULL, sizeof(struct fifo), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
	fifo_init(f);
	int i,p; int j = 0;
	int readers = 4; int writers = 3;
	int readers2pid[readers], writers2pid[writers];
	unsigned long d;
	for(i=0; i<readers; i++){ /* Readers */
		p=fork();
		if(p==-1){perror("fork"); return -1;}
		else if(!p){
			while(j<3){
				d = fifo_rd(f);
				fprintf(stderr, "P-%d read %lu\n",i,d);
			}
			break;
		}
		readers2pid[i]=p;
	}
	if(i==readers){//Lookup Table
		fprintf(stderr, "Readers:\n");
		for(j=0; j<readers;j++){
			fprintf(stderr,"procnum[%d]=process %d\n",j,readers2pid[j]);
		}
		for(i=0; i<writers; i++){
			p=fork();
			if(p==-1){perror("Fork"); return -1;}
			else if(!p){
				while(j<4){
					d = j;
					fifo_wr(f,d);
					fprintf(stderr, "P-%d wrote %lu\n",i,d);
				}
				break;
			}
			writers2pid[i] = p;
		}
		if(i==writers){
			fprintf(stderr, "Writers:\n");
			for(j=0;j<writers;j++){
				fprintf(stderr, "procnum[%d]=process %d\n", j, writers2pid[j]);
			}				
		}
	}
	return 0;
}
