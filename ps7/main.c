/* Mark Bryk OS PS7 MAIN.C */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "fifo.h"

#define N_PROC 64
sigset_t mask;
static void handler(int sn){ }
int main(int argc, char **argv){
	/* For readers not to hang, readers*readBytes <= writers*writeBytes */
	int readers = 1; int writers = 5;
	int readBytes = 15; int writeBytes = 3;
	if((readers+writers)>N_PROC){fprintf(stderr, "Error - Too Many Processes Opened\n"); return -1;}

	sigfillset(&mask);
	sigdelset(&mask,SIGUSR1); /* Mask is an extern which can be accessed by every process from sem.c */
	signal(SIGUSR1, handler);

	struct fifo *f = mmap(NULL, sizeof(struct fifo), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
	if(f==MAP_FAILED){ perror("Error Mapping Fifo to Memory"); return -1;}
	fifo_init(f);

	int i,p; int j = 0;
	int pn2pid[readers+writers]; /* Lookup Table For Debugging Prints */
	unsigned long d;
	for(i=1; i<=readers; i++){
		p=fork();
		if(p==-1){perror("Error Forking a Reader"); return -1;}
		else if(!p){
			for(j=0;j<readBytes;j++){
				p = getpid();
				d = fifo_rd(f);
				fprintf(stderr, "\t\t\tReader #%d = %lu\n",i,d);
			}
			return 0;
		}
		pn2pid[i]=p;
	}
	for(i=1; i<=writers; i++){
		p=fork();
		if(p==-1){perror("Error Forking a Writer"); return -1;}
		else if(!p){
			for(j=1;j<=writeBytes;j++){
				p = getpid();
				d = i*100+j;
				fifo_wr(f,d);
				fprintf(stderr, "Writer #%d = %lu\n",i,d);
			}
			return 0;
		}
		pn2pid[readers+i] = p;
	}
	/* LOOKUP TABLE
	for(j=0;j<readers+writers;j++){
		fprintf(stderr, "Procnum[%d] = Process #%d\n", j, pn2pid[j]);
	}
	*/

	int stat;
	for(i=0;i<readers+writers;i++){
		if(wait(&stat)==-1){ perror("Error Returning from Child Process"); return -1;}
	}
	return 0;
}
