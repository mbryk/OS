/* Mark Bryk OS PS9 SCHED.H */
#ifndef SCHED_H
#define SCHED_H
#include <sys/types.h>

#define SCHED_NPROC		256
#define SCHED_READY		0
#define SCHED_RUNNING	1
#define SCHED_SLEEPING	2
#define SCHED_ZOMBIE	3

struct sched_proc{
	pid_t pid;
	int proc;
	int nice;
	void *stack;
	int state;
	int vruntime; // ?
};
struct sched_waitq{
	struct sched_proc *proc;
	void (*func)();
	struct sched_waitq *next;
};

struct sched_proc *current;

void sched_init(void (*init_fn)());
int sched_fork();
void sched_exit(int);
void sched_wait(int*);
void sched_nice(int); // -1=Error. 0=Success
pid_t sched_getpid();
pid_t sched_getppid();
int sched_gettick();
void sched_ps(); // Sighandler
void sched_switch();
void sched_tick(); // Sighandler

#endif

