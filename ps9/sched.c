#include "sched.h"
#include <unistd.h> //getpid()
#include <sys/time.h> //setitimer
#include <stdio.h>
#include <signal.h>
#include <string.h> //memset
#include <sys/mman.h> //mmap
#include <math.h> // pow
#include <stdlib.h> //abs, exit

sched_init(void (*init_fn)()){
	sigemptyset(&set);
	sigaddset(&set,SIGABRT);sigaddset(&set,SIGVTALRM);sigaddset(&set,SIGUSR1);sigaddset(&set,SIGUSR2);
	sigprocmask(SIG_BLOCK,&set,NULL);

	/*** GLOBAL VARS ****/
	totalticks = 0; runticks = 0; quantumticks = 100; // TICKS PER PROCESS
	resched = 0; pids = 0;
	totalexited = 0;
	memset(exited,0, SCHED_NPROC*sizeof(struct sched_proc*));	

	/**** TIMER *****/
	struct itimerval *timer = malloc(sizeof(struct itimerval));
	if(timer==NULL){ perror("malloc failed\n"); exit(-1);}
	struct timeval tv;
	tv.tv_usec = 1000; // 1ms
	tv.tv_sec = 0;
	timer->it_interval = tv;
	timer->it_value = tv;

	/**** MAKE INIT PROCESS *****/
	init = malloc(sizeof(struct sched_proc));
	if(init==NULL){ perror("malloc failed\n"); exit(-1);}
	sched_proc_init(init);
	memset(init->stack,0,STACK_SIZE);
	init->state = SCHED_RUNNING;
	current = init;

	/*** MAKE RUN QUEUE ****/
	rq = malloc(sizeof(struct sched_waitq));
	if(rq==NULL){ perror("malloc failed\n"); exit(-1);}
	sched_waitq_init(rq);

	if(setitimer(ITIMER_VIRTUAL, timer, NULL)<0){ perror("timer could not be set\n"); exit(-1);}
	if(signal(SIGVTALRM,sched_tick)==SIG_ERR){ perror("timer sighandler could not be set"); exit(-1);}
	if(!savectx((&current->context))){
		current->context.regs[JB_SP] = current->stack+STACK_SIZE;
		current->context.regs[JB_BP] = current->stack+STACK_SIZE;
		current->context.regs[JB_PC] = init_fn;
		sched_switch();
	}
}

int sched_proc_init(struct sched_proc *p){
	if(totalexited){
		p->pid = exited[--totalexited]->pid; // Reuse PID's of the exited processes which are no longer Zombies.
	} else {
		if(pids==SCHED_NPROC) return -1;
		p->pid = ++pids;
	}
	p->nice = 0; p->vruntime = 0;
	p->state = SCHED_READY;

	/*** MAKE STACK ****/
	if((p->stack=mmap(0,STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,0,0))==MAP_FAILED)
	{
		perror("mmap failed"); return -1;
	}
	p->parent = NULL; p->exit_code = 0;
	memset(p->children,0,SCHED_NPROC*sizeof(struct sched_proc*));
	p->childcount = 0;
	return 0;
}

int sched_fork(){
	sigprocmask(SIG_BLOCK,&set,NULL);

	struct sched_proc *new = malloc(sizeof(struct sched_proc));
	if(new==NULL){ perror("malloc failed\n"); return -1;}
	if(sched_proc_init(new)<0) return -1;
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
		}
		adjstack(new->stack,new->stack+STACK_SIZE,diff)
		new->context.regs[JB_SP] += diff;
		new->context.regs[JB_BP] += diff;
		sched_switch();
	}
}

sched_exit(int code){
	sigprocmask(SIG_BLOCK,&set,NULL);
	if(current->pid == 1){
		fprintf(stderr, "Exit Init\nShutting Down...\n");
		exit(code);
	}
	current->state = SCHED_ZOMBIE;
	current->exit_code = code;
	if(current->parent->state == SCHED_WAITING){
		current->parent->state = SCHED_READY;
		heap_insert(rq, current->parent);
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
			return child->pid;
		}
	}

	// All children alive and well, Thank god. All Doctors and Lawyers. Except for one who went into education. Oy.
	current->state = SCHED_WAITING;
	while(savectx(&(current->context))){ // While loop = just in case none of them are zombies and I was woken up. Should never happen.
		for(i=0;i<current->childcount;i++){
			if(current->children[i]->state==SCHED_ZOMBIE){
				struct sched_proc *child = current->children[i];
				*exit_code = child->exit_code;
				exited[totalexited++] = child;
				current->children[i] = current->children[--current->childcount];
				return child->pid;
			}
		}
	}
	sched_switch();	
}

sched_nice(int niceval){
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

sched_ps(){
	sigprocmask(SIG_BLOCK,&set,NULL);
	printf("pid\tppid\tstate\tstack\t\tnice\tweight\tticks\n");
	sched_printchildren(init);
	printf("Total CPU Ticks: %d\n",sched_gettick());
}

sched_printchildren(struct sched_proc *p){
	double weight;
	weight = pow(1.25,(double)p->nice);
	int myticks = p->vruntime/weight;

	printf("%d \t",p->pid);
	if(p->pid!=1) printf("%d",p->parent->pid);
	
	printf("\t%d \t%p \t%d \t%.3f \t%d\n",p->state,p->stack+STACK_SIZE,p->nice,weight,myticks);

	int i;	
	for(i=0;i<p->childcount;i++){
		sched_printchildren(p->children[i]);
	}
}

sched_sleep(struct sched_waitq *wq){
	sigprocmask(SIG_BLOCK,&set,NULL);
	current->state = SCHED_SLEEPING;
	heap_insert(wq, current);
	if(!savectx(&(current->context)))
		sched_switch();
}

sched_switch(){
	if(current->state==SCHED_RUNNING){
		current->state = SCHED_READY;
		heap_insert(rq, current);
	}
	if(!rq->filled){ // In a case where everybody is sleeping and waiting for a signal
		sigprocmask(SIG_UNBLOCK,&set,NULL);
		for(;;); // Idle Task until interrupted by a signal.
	}
	else if(rq->filled==1) current = rq->queue[rq->filled--];
	else	current = heap_deleteMin(rq);
	current->state = SCHED_RUNNING;
	sigprocmask(SIG_UNBLOCK,&set,NULL);
	restorectx(&(current->context),1);
}

sched_tick(){
	sigprocmask(SIG_BLOCK,&set,NULL);

	totalticks++; runticks++;
	double weight = pow(1.25,(double)current->nice);
	current->vruntime += weight;
	if(runticks > quantumticks || resched){
		runticks = 0;
		resched = 0;
		if(!savectx(&(current->context)))
			sched_switch();
	} else
		sigprocmask(SIG_UNBLOCK,&set,NULL);
}

sched_wakeup(struct sched_waitq *wq){
	sigprocmask(SIG_BLOCK,&set,NULL);

	int i;
	struct sched_proc *p;
	if((i=wq->filled)>0) resched = 1;
	for(;i>0;i--){ // Wake up entire wait queue
		p = wq->queue[wq->filled--];
		p->state = SCHED_READY;
		heap_insert(rq,p);
	}

	sigprocmask(SIG_UNBLOCK,&set,NULL);
}

sched_waitq_init(struct sched_waitq *wq){
	wq->filled = 0;
	memset(wq->queue,0, (SCHED_NPROC+1)*sizeof(struct sched_proc*));
}

heap_insert(struct sched_waitq *wq, struct sched_proc *proc){
	int nextPos = ++wq->filled;
	wq->queue[nextPos] = proc;
	heap_percolateUp(wq, nextPos);
}

void *heap_deleteMin(struct sched_waitq *wq){
	void *ret = wq->queue[1];
	wq->queue[1] = wq->queue[wq->filled--];
	heap_percolateDown(wq, 1);
	return ret;
}

heap_percolateUp(struct sched_waitq *wq, int index){
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

heap_percolateDown(struct sched_waitq *wq, int index){
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

adjstack(void *lim0,void *lim1,long adj)
{
 void **p;
 void *prev,*new;
#ifdef _LP64
       __asm__(
	"movq   %%rbp,%0"
	:"=m" (p));
#else
       __asm__(
	"movl   %%ebp,%0"
	:"=m" (p));
#endif
	/* Now current bp (for adjstack fn) is in p */
	/* Unwind stack to get to saved ebp addr of caller */
	/* then begin adjustment process */
	fprintf(stderr,"Asked to adjust child stack by %#lX bytes bet %p and %p\n",adj,lim0,lim1);
	prev=*p;
	p= prev + adj;
	for(;;)
	{
		prev=*p;
		new=prev+adj;
		if (new<lim0 || new>lim1)
		{
			fprintf(stderr,"Enough already, saved BP @%p is %p\n",
						p,prev);
			break;
		}
		*p=new;
		fprintf(stderr,"Adjusted saved bp @%p to %p\n",
				p,*p);
		p=new;
	}
}
