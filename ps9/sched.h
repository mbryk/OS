/* Mark Bryk OS PS9 SCHED.H */
#ifndef SCHED_H
#define SCHED_H
#include <sys/types.h>
#include "savectx64.h"

#define SCHED_NPROC		256
#define SCHED_READY		0
#define SCHED_RUNNING	1
#define SCHED_SLEEPING	2
#define SCHED_WAITING	3
#define SCHED_ZOMBIE	4

#define STACK_SIZE		65536 // 64K

struct sched_proc{
	int pid;
	int nice;
	void *stack;
	int state;
	double vruntime;
	struct sched_proc *parent; int exit_code;
	struct sched_proc *children[SCHED_NPROC]; int childcount;
	struct savectx context;
};
struct sched_waitq{
	int filled;
	struct sched_proc *queue[SCHED_NPROC+1]; // My Implementation has no 0th entry.
};

/*** Scheduler Global Variables ***/
int totalticks, runticks, quantumticks, resched;
sigset_t set; int pids;
// init is a global var so that sched_ps can iterate through the tree of all processes on the system
struct sched_proc *current, *init; 
struct sched_waitq *rq;
struct sched_proc *exited[SCHED_NPROC]; int totalexited;

sched_init(void (*init_fn)());
int sched_proc_init(struct sched_proc *p);
int sched_fork();
sched_exit(int);
int sched_wait(int*); 
sched_nice(int);
int sched_getpid();
int sched_getppid();
int sched_gettick();
sched_ps();
sched_sleep(struct sched_waitq*);
sched_switch();
sched_tick();
sched_waitq_init(struct sched_waitq*);
heap_insert(struct sched_waitq*, struct sched_proc *proc);
void *heap_deleteMin(struct sched_waitq*);
heap_percolateUp(struct sched_waitq*, int);
heap_percolateDown(struct sched_waitq*, int);

#endif