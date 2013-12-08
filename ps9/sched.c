#include "sched.h"
#include <unistd.h> //getpid()
#include <sys/time.h> //setitimer
#include <stdio.h>
#include <signal.h>
#include <string.h> //memset
#include <stdlib.h> //malloc

void sched_init(void (*init_fn)()){
	
	/*** TIMER ****/
	totalticks = 0; runticks = 0; 
	quantumticks = 10; // TICKS PER PROCESS

	struct itimerval timer;
	struct timeval tv;
	tv.tv_usec = 100; 
	tv.tv_sec = 0;
	timer.it_interval = tv;
	timer.it_value = tv;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
	signal(SIGVTALRM,sched_tick);
	/*** /TIMER ****/

	/**** MAKE INIT PROCESS *****/
	struct sched_proc init;
	sched_proc_init(&init);
	current = &init;
	/**** /MAKE INIT PROCESS*****/

}

void sched_proc_init(struct sched_proc *p){
	p->pid = current->pid + 1;
	p->nice = 0;
	p->state = SCHED_READY;
	p->vruntime = current->vruntime;
	sched_calcWeights();

	/*** MAKE STACK ****/
	void *stack = malloc(STACK_SIZE);
	if(stack==NULL) { perror("malloc failed"); exit(-1); }
	adjstack();
	p->stack = stack;
	/*** /STACK ****/

	p->parent = NULL;
}

void sched_calcWeights(){

}

int sched_fork(){
	struct sched_proc new;
	sched_proc_init(&new);
	new.parent = current;
	heap_insert(rq,&new);

	if(savectx(new->context)){
		return 0; // CHILD
	}
	if(savectx(current->context)){
		return new->pid;
	} else {
		sched_switch();
	}
}

void sched_exit(int code){

}

void sched_wait(int *exit_code){

}

void sched_nice(int niceval){
	if(niceval>19) niceval = 19;
	else if(niceval<-20) niceval = -20;
	current->nice = niceval;
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

void sched_ps(){ // ABORT Sighandler
	fprintf(stderr,"UHOH. ABORT ABORT!\n");
}

void sched_sleep(struct sched_waitq *wq){
	current->state = SCHED_SLEEPING;
	heap_insert(wq, current);
	if(!savectx(&(current->context))
		sched_switch();
	// else, return from sleep command.
}

void sched_switch(){
	if(current->state==SCHED_RUNNING){
		current->state = SCHED_READY;
		heap_insert(rq, current);
	}
	current = heap_deleteMin(rq);
	current->state = SCHED_RUNNING;
	restorectx(&(current->context),1);
}

void sched_tick(){ // Sighandler
	totalticks++; runticks++;
	current->vruntime += 1/current->weight;
	if(runticks > quantumticks){
		runticks = 0;
		sched_switch();
	}
}

void sched_wakeup(struct sched_waitq *wq){ // ON EVENT Defined by Wait Queue
	int i;
	struct sched_proc *p;
	for(i=wq->filled;i>0;i++){
		p = wq->queue[wq->filled--];
		p->state = SCHED_READY;
		heap_insert(rq,p);
	}
}

void sched_waitq_init(struct sched_waitq *wq){
	wq->filled = 0;
	memset(wq->queue,0, SCHED_NPROC);
}

void heap_insert(struct sched_waitq *wq, struct sched_proc *proc){
	int nextPos = ++wq->filled;
	
	// Just checking. Should never be able to happen.
	if(nextPos > SCHED_NPROC) {fprintf(stderr,"Uhoh. Queue too full\n"); exit(-1);}

	wq->queue[nextPos] = proc;
	heap_percolateUp(wq, nextPos);
}

void *heap_deleteMin(struct sched_waitq *wq){
	if(!wq->filled){ fprintf(stderr, "Waitq is empty...?"); return NULL;}
	void *ret = wq->queue[1];
	wq->queue[1] = wq->queue[filled--];
	heap_percolateDown(wq, 1);
	return ret;
}

void heap_percolateUp(struct sched_waitq *wq, int index){
	int changed = 0;
	struct sched_proc *temp = wq->queue[index];

	while((index>1) && (temp->vruntime < wq->queue[index/2]->key)){
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
	for(; index*2 <= wq->qtot; index=child){
		child = index*2; // Next Layer
		
		if(child != wq->qtot && wq->queue[child+1]->vruntime < wq->queue[child]->vruntime) // Look down both nodes
			child++;

		if(wq->queue[child]->vruntime < temp->vruntime) {
			wq->queue[index] = wq->queue[child];
		} else break;
	}
	wq->queue[index] = temp;
}


/*adjstack takes a stack which is bounded by addresses lim0 and lim1
 *and adjusts the saved frame pointers (ebp) by adj bytes.
 *It is intended to be used in conjunction with sched_fork to fix
 *the child's stack, and should be called from the parent task, after
 *having allocated the child's stack and copying into it the contents of
 *the entire parent's stack.  There are some debugging fprintfs that
 *could be commented out if needed.
 */

adjstack(void *lim0,void *lim1,unsigned long adj)
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