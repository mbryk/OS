/* Mark Bryk OS PS7 MAIN.C */
#include <stdlib.h>
#include <stdio.h>
#include "fifo.h"

#define N_PROC 64
extern int proc_num;
extern int lookup[N_PROC];
int main(int argc, char **argv){
	int i, p;
	struct fifo f = malloc(sizeof(struct fifo));
	if(f==NULL){ perror("malloc"); return -1; }
	
	for(i=0; i<N_PROC; i++){
		switch(p=fork()){
			case -1:
				perror("fork"); return -1;
			case 0:	
				proc_num = i; break;
			default:
				lookup[i] = p;
				break;
		}
	}
	return 0;
}
