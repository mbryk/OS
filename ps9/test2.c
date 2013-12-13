#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h> // exit
#include <time.h>
#include "sched.h"
#define DELAY_FACTOR 31
init_fn(){
	int i,p;
	time_t t = time(0);
	for(i=0;i<5;i++){
		p=sched_fork();
		if(p<0){ fprintf(stderr,"fork #%d failed\n",i); exit(-1); }
		else if(!p){ sched_nice((i-2)*3); break; }
	}
	unsigned long int x;
	for(x=1;x<1UL<<DELAY_FACTOR;x++)
		;
	if(current->pid==1){
		kill(getpid(),SIGABRT);
	} else {
		sched_exit(0);
	}
	struct sched_waitq wq1;
	sched_waitq_init(&wq1);
	sched_sleep(&wq1); // Sleep Indefinitely
}
abrt_handler(){
	sched_ps();
}
main(){
	struct sigaction sa;
	fprintf(stderr,"Starting\n");
	sa.sa_flags=0;
	sa.sa_handler=abrt_handler;	sigaction(SIGABRT,&sa,NULL); 
	sched_init(init_fn);
}