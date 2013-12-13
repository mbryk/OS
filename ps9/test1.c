#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h> // exit
#include <time.h>
#include "sched.h"

#define DELAY_FACTOR 29
struct sched_waitq wq1,wq2;

init_fn()
{
	sched_waitq_init(&wq1);
	sched_waitq_init(&wq2);
	int x;
	fprintf(stderr,"Hooray made it to init_fn, stkaddr %p\n",&x);
	switch (sched_fork())
	{
	 case -1:
		fprintf(stderr,"fork failed\n");
		exit(-1);
	 case 0:
		child_fn1();
		fprintf(stderr,"!!BUG!! at %s:%d\n",__FILE__,__LINE__);
	 default:
		parent_fn();
		break;
	}
	fprintf(stderr,"DONE!\n");
	kill(getpid(),SIGABRT);
	exit(0);
}

child_fn1()
{
	int x;
	fprintf(stderr,"<<in child 1 addr %p>>\n",&x);
	fprintf(stderr,"Child 1 - Go to sleep on wq1\n");
	sched_sleep(&wq1);
	fprintf(stderr,"Child 1 - Woken Up\n");
	for(x=1;x<1<<DELAY_FACTOR;x++)
		;
	fprintf(stderr,"Child 1 - exit(22)\n");
	kill(getpid(),SIGABRT);
	sched_exit(22);
}

parent_fn()
{
 	int y,p;
 	pid_t pid = getpid();
 	fprintf(stderr,"<<in parent addr %p>>\n",&y);
	switch(sched_fork())
	{
	 case -1:
	 	fprintf(stderr,"Fork failed\n");
		return;
	 case 0:
		child_fn2();
		fprintf(stderr,"Child 2 - exit(11)\n");
		sched_exit(11);
		fprintf(stderr,"!!BUG!! at %s:%d\n",__FILE__,__LINE__);
		return;
	 default:
	 	fprintf(stderr,"Parent - Wake up wq1 & wq2\n");
	 	kill(pid,SIGUSR1);//wq1
	 	kill(pid,SIGUSR2);//wq2
		while ((p=sched_wait(&y))>0)
			fprintf(stderr,"Waited for child pid %d return code %d\n",p,y);
		return;
	}
}

child_fn2()
{
	sched_nice(4);
	int x;
	fprintf(stderr,"<<in child 2 addr %p>>\n",&x);
	fprintf(stderr,"Child 2 - Go to sleep on wq2\n");
	sched_sleep(&wq2);
	fprintf(stderr,"Child 2 - Woken Up\n");
	for(x=1;x<1<<DELAY_FACTOR;x++)
		;
	kill(getpid(),SIGABRT);
}

wakeup_handler(int sig)
{
	if (sig==SIGUSR1)
		sched_wakeup(&wq1);
	else
		sched_wakeup(&wq2);
}

abrt_handler(int sig)
{
	sched_ps();
}

main()
{
 	struct sigaction sa;
	fprintf(stderr,"Starting\n");
	sa.sa_flags=0;
	sa.sa_handler=wakeup_handler; sigemptyset(&sa.sa_mask); sigaction(SIGUSR1,&sa,NULL); sigaction(SIGUSR2,&sa,NULL);
	sa.sa_handler=abrt_handler;	sigaction(SIGABRT,&sa,NULL); 

	sched_init(init_fn);
	fprintf(stderr,"Whoops. Init returned\n");
}
