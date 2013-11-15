/* Mark Bryk OS PS7 MAIN.C */
#include <stdlib.h>

#include "fifo.h"

#define N_PROC 64
extern int proc_num;
int main(int argc, char **argv){
	int i;
	struct fifo f = malloc(sizeof(struct fifo));
	if(f==NULL){ perror("malloc"); return -1; }
	for(i=0; i<N_PROC; i++){
		switch(fork()){
			case -1:
				perror("fork"); return -1;
			case 0:	
				proc_num = i; break;
			default:
				break;
		}
	}
	return 0;
}
