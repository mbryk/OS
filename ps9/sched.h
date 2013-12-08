/* Mark Bryk OS PS9 SCHED.H */
#ifndef SCHED_H
#define SCHED_H
#include <sys/types.h>
#include "savectx.h"

#define SCHED_NPROC		256
#define SCHED_READY		0
#define SCHED_RUNNING	1
#define SCHED_SLEEPING	2
#define SCHED_ZOMBIE	3

#define STACK_SIZE		65536 // 64K

struct sched_proc{
	int pid;
	int nice; int weight;
	void *stack;
	int state;
	int vruntime;
	struct sched_proc *parent;
	struct savectx context;
};
struct sched_waitq{
	int filled;
	struct sched_proc *queue[SCHED_NPROC];
};

/*** Scheduler Variables ***/
int totalticks, runticks, quantumticks;
struct sched_proc *current;
struct sched_waitq *rq;

void sched_init(void (*init_fn)());
void sched_waitq_init(struct sched_waitq*);
int sched_fork();
void sched_exit(int);
void sched_wait(int*);
void sched_nice(int);
int sched_getpid();
int sched_getppid();
int sched_gettick();
void sched_ps(); // Sighandler
void sched_sleep(struct sched_waitq*);
void sched_switch();
void sched_tick(); // Sighandler

adjstack(void *lim0,void *lim1,unsigned long adj);
void heap_insert(struct sched_waitq*, void *proc, int val);
void *heap_deleteMin(struct sched_waitq*);
void heap_percolateUp(int);
void heap_percolateDown(int);

#endif