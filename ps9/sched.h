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
	struct sched_proc *queue[SCHED_NPROC];
};

/*** Scheduler Variables ***/
int totalticks, runticks, quantumticks;
sigset_t set;
struct sched_proc *current;
struct sched_waitq *rq;
int pids; int totalexited;
struct sched_proc *exited[SCHED_NPROC];

void sched_init(void (*init_fn)());
void sched_proc_init(struct sched_proc *p);
void sched_waitq_init(struct sched_waitq*);
int sched_fork();
void sched_exit(int);
int sched_wait(int*); 
void sched_nice(int);
int sched_getpid();
int sched_getppid();
int sched_gettick();
void sched_ps(); // Sighandler
void sched_sleep(struct sched_waitq*);
void sched_switch();
void sched_tick(); // Sighandler

void adjstack(void *lim0,void *lim1,long adj);
void heap_insert(struct sched_waitq*, struct sched_proc *proc);
void *heap_deleteMin(struct sched_waitq*);
void heap_percolateUp(struct sched_waitq*, int);
void heap_percolateDown(struct sched_waitq*, int);

#endif