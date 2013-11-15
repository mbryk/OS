/* Mark Bryk OS PS7 SEM.C */
#include "sem.h"

sigset_t mask;
void sem_init(struct sem *s, int count){
	s->lock = 0;
	s->count = count;
	s->qnext = 0;
	s->qtot = 0;
	memset(s->queue, 0, QSIZE);
}

int sem_try(struct sem *s){
	int l = 0;
	while(tas(&s->lock)!=0);
	if(s->count==1){
		l = 1;
		s->count--;
	}
	s->lock = 0;
	return l;		
}
static void handler(int sn){
	printf("handled\n");
}

void sem_wait(struct sem *s){
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask,SIGUSR1);
	signal(SIGUSR1,handler);
	s->count--;
	while(1){
		while(tas(&s->lock)!=0); // Grab lock for count check
		if(s->count < 0){ // Somebody is using semaphore
			s->queue[s->qtot++] = proc_num;
			s->qtot %= QSIZE;
			s->lock = 0; // Release lock
			sigsuspend(&mask);
			continue;	
		}
		s->lock = 0;
		break; //Now you have lock and semaphore says that you're good
	}
}
void sem_inc(struct sem *s){
	s->count++;
	while(tas(&s->lock)!=0); // Grab lock for count check
	int c = s->count;
	s->lock = 0;
	int q;
	if(c <= 0){ // There are people waiting
		q = s->qnext;
		s->qnext = (s->qnext+1)%QSIZE;	
		kill(lookup[q],SIGUSR1);
	}
}
