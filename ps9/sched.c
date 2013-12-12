#include "sched.h"
#include <unistd.h> //getpid()
#include <sys/time.h> //setitimer
#include <stdio.h>
#include <signal.h>
#include <string.h> //memset
#include <sys/mman.h> //mmap
#include <math.h> // pow
#include <stdlib.h> //abs

void sched_init(void (*init_fn)()){
	sigemptyset(&set);
	sigaddset(&set,SIGABRT);sigaddset(&set,SIGVTALRM);sigaddset(&set,SIGUSR1);sigaddset(&set,SIGUSR2);
	sigprocmask(SIG_BLOCK,&set,NULL);

	/*** GLOBAL VARS ****/
	totalticks = 0; runticks = 0; 
	quantumticks = 10; // TICKS PER PROCESS
	pids = 0;
	totalexited = 0;
	memset(exited,0, SCHED_NPROC*sizeof(struct sched_proc*));	

	/**** TIMER *****/
	struct itimerval *timer = malloc(sizeof(struct itimerval));
	struct timeval tv;
	tv.tv_usec = 10; 
	tv.tv_sec = 0;
	timer->it_interval = tv;
	timer->it_value = tv;

	/**** MAKE INIT PROCESS *****/
	struct sched_proc *init = malloc(sizeof(struct sched_proc));
	sched_proc_init(init);
	memset(init->stack,0,STACK_SIZE);
	init->state = SCHED_RUNNING;
	current = init;

	/*** MAKE RUN QUEUE ****/
	rq = malloc(sizeof(struct sched_waitq));
	sched_waitq_init(rq);

	setitimer(ITIMER_VIRTUAL, timer, NULL);
	signal(SIGVTALRM,sched_tick);
	if(!savectx((&current->context))){
		current->context.regs[JB_SP] = current->stack+STACK_SIZE;
		current->context.regs[JB_BP] = current->stack+STACK_SIZE;
		current->context.regs[JB_PC] = init_fn;
		sched_switch();
	}
}

void sched_proc_init(struct sched_proc *p){
	p->pid = ++pids;
	p->nice = 0;
	p->state = SCHED_READY;

	/*** MAKE STACK ****/
	void *newsp;
	if((newsp=mmap(0,STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,0,0))==MAP_FAILED)
	{
		perror("mmap failed");
	}
	p->stack = newsp;
	p->parent = NULL;
	memset(p->children,0,SCHED_NPROC*sizeof(struct sched_proc*));
	p->childcount = 0;
}

int sched_fork(){
	sigprocmask(SIG_BLOCK,&set,NULL);

	struct sched_proc *new = malloc(sizeof(struct sched_proc));
	sched_proc_init(new);
	new->parent = current;
	new->vruntime = current->vruntime;
	current->children[current->childcount++] =  new;

	heap_insert(rq,new);
	long diff = new->stack-current->stack;
	memcpy(new->stack,current->stack,STACK_SIZE);
	if(savectx(&(current->context))){
		return new->pid;
	} else {
		if(savectx(&(new->context))){
			return 0;
		} else {
			new->context.regs[JB_SP] += diff;
			new->context.regs[JB_BP] += diff;
		}
		sched_switch();
	}
}

void sched_exit(int code){
	sigprocmask(SIG_BLOCK,&set,NULL);

	if(current->parent->state == SCHED_WAITING){
		current->state = SCHED_ZOMBIE;
		current->exit_code = code;
		heap_insert(rq, current->parent);
	} else {
		exited[totalexited++] = current;
	}
	sched_switch();
}

int sched_wait(int *exit_code){
	sigprocmask(SIG_BLOCK,&set,NULL);

	//No Children
	if(!current->childcount){
		sigprocmask(SIG_UNBLOCK,&set,NULL);
		return -1;
	}

	// Child already a Zombie
	int i;
	for(i=0;i<current->childcount;i++){
		if(current->children[i]->state==SCHED_ZOMBIE){
			struct sched_proc *child = current->children[i];
			*exit_code = child->exit_code;
			exited[totalexited++] = child;
			current->children[i] = current->children[--current->childcount];
			sigprocmask(SIG_UNBLOCK,&set,NULL);
			return 0;
		}
	}

	// All children alive and well, Thank god. All Doctors and Lawyers. Except for one who went into education. Oy.
	current->state = SCHED_WAITING;
	while(savectx(&(current->context))){ // Just in case none of them are zombies and I was woken up. Should never happen.
		for(i=0;i<current->childcount;i++){
			if(current->children[i]->state==SCHED_ZOMBIE){
				struct sched_proc *child = current->children[i];
				*exit_code = child->exit_code;
				exited[totalexited++] = child;
				current->children[i] = current->children[--current->childcount];
				current->state = SCHED_RUNNING;
				return 0;
			}
		}
	}
	sched_switch();	
}

void sched_nice(int niceval){
	sigprocmask(SIG_BLOCK,&set,NULL);

	if(niceval>19) niceval = 19;
	else if(niceval<-20) niceval = -20;
	current->nice = niceval;

	sigprocmask(SIG_UNBLOCK,&set,NULL);
}

int sched_getpid(){
	return current->pid;
}

int sched_getppid(){
	return current->parent->pid;
}

int sched_gettick(){
	return totalticks;
}

void sched_ps(){
	fprintf(stderr,"UHOH. ABORT ABORT!\n");
}

void sched_sleep(struct sched_waitq *wq){
	sigprocmask(SIG_BLOCK,&set,NULL);

	current->state = SCHED_SLEEPING;
	heap_insert(wq, current);
	if(!savectx(&(current->context)))
		sched_switch();
}

void sched_switch(){
	if(current->state==SCHED_RUNNING){
		current->state = SCHED_READY;
		heap_insert(rq, current);
	}
	if(rq->filled==1) current = rq->queue[rq->filled--];
	else	current = heap_deleteMin(rq);
	current->state = SCHED_RUNNING;
	sigprocmask(SIG_UNBLOCK,&set,NULL);
	restorectx(&(current->context),1);
}

void sched_tick(){
	sigprocmask(SIG_BLOCK,&set,NULL);

	fprintf(stderr, "Tick ");
	totalticks++; runticks++;
	double weight = pow(1.25,(double)current->nice);
	current->vruntime += weight;
	if(runticks > quantumticks){
		runticks = 0;
		if(!savectx(&(current->context)))
			sched_switch();
	} else
		sigprocmask(SIG_UNBLOCK,&set,NULL);
}

void sched_wakeup(struct sched_waitq *wq){
	sigprocmask(SIG_BLOCK,&set,NULL);

	int i;
	struct sched_proc *p;
	for(i=wq->filled;i>0;i--){
		p = wq->queue[wq->filled--];
		p->state = SCHED_READY;
		heap_insert(rq,p);
	}

	sigprocmask(SIG_UNBLOCK,&set,NULL);
}

void sched_waitq_init(struct sched_waitq *wq){
	wq->filled = 0;
	memset(wq->queue,0, SCHED_NPROC*sizeof(struct sched_proc*));
}

void heap_insert(struct sched_waitq *wq, struct sched_proc *proc){
	int nextPos = ++wq->filled;
	
	if(nextPos > SCHED_NPROC) {fprintf(stderr,"Uhoh. Queue too full\n"); exit(-1);}

	wq->queue[nextPos] = proc;
	heap_percolateUp(wq, nextPos);
}

void *heap_deleteMin(struct sched_waitq *wq){
	if(!wq->filled){ fprintf(stderr, "Waitq is empty...?"); return NULL;}
	void *ret = wq->queue[1];
	wq->queue[1] = wq->queue[wq->filled--];
	heap_percolateDown(wq, 1);
	return ret;
}

void heap_percolateUp(struct sched_waitq *wq, int index){
	int changed = 0;
	struct sched_proc *temp = wq->queue[index];

	while((index>1) && (temp->vruntime < wq->queue[index/2]->vruntime)){
		wq->queue[index] = wq->queue[index/2];
		index /= 2;
		changed = 1;
	}
	if(changed){
		wq->queue[index] = temp;
	}
}

void heap_percolateDown(struct sched_waitq *wq, int index){
	struct sched_proc *temp = wq->queue[index];
	int child;
	for(; index*2 <= wq->filled; index=child){
		child = index*2; // Next Layer
		
		if(child != wq->filled && wq->queue[child+1]->vruntime < wq->queue[child]->vruntime) // Look down both nodes
			child++;

		if(wq->queue[child]->vruntime < temp->vruntime) {
			wq->queue[index] = wq->queue[child];
		} else break;
	}
	wq->queue[index] = temp;
}