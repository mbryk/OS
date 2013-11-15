/* Mark Bryk OS PS7 SEM.H */
#ifndef SEM_H
#define SEM_H

#include <signal.h>

#define QSIZE 64
int tas(volatile int*);
struct sem{
	int lock;
	int count;	
	int qnext, qtot;
	int queue[QSIZE];
};
void sem_init(struct sem*,int);
int sem_try(struct sem*);
void sem_wait(struct sem*);
void sem_inc(struct sem *);

#endif
