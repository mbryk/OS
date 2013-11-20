/* Mark Bryk OS PS7 SEM.C */
#include "sem.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

extern sigset_t mask;

void sem_init(struct sem *s, int count){
	s->lock = 0;
	s->count = count;
	s->qnext = 0;
	s->qtot = 0;
	memset(s->queue,0, QSIZE);
}

int sem_try(struct sem *s){
	int l = 0;
	while(tas(&s->lock)!=0);
	if(s->count>0){
		l = 1;
		s->count--;
	}
	s->lock = 0;
	return l;		
}
void sem_wait(struct sem *s){
	while(1){
		while(tas(&s->lock)!=0);
		if(s->count > 0){
			s->count--;
			s->lock = 0;
			break;
		}
		s->queue[s->qtot++] = getpid();
		s->qtot %= QSIZE;
		s->lock = 0;
		sigsuspend(&mask);
	}
}
void sem_inc(struct sem *s){
	while(tas(&s->lock)!=0);
	if(++s->count>0 && s->qnext!=s->qtot){ /* There is room and there are people waiting */
		kill(s->queue[s->qnext],SIGUSR1);
		s->qnext = (s->qnext+1)%QSIZE;	
	}
	s->lock = 0;
}
