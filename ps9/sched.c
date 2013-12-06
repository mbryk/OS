#include "sched.h"
#include <unistd.h> //getpid()
#include <sys/time.h> //setitimer
#include <stdio.h>
#include <signal.h>

void sched_init(void (*init_fn)()){
	struct itimerval timer;
	struct timeval tv;
	tv.tv_usec = 0; tv.tv_sec = 1;
	timer.it_interval = tv;
	timer.it_value = tv;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
	signal(SIGVTALRM,sched_tick);

}

int sched_fork(){

	return 0;
}

void sched_exit(int code){

}

void sched_wait(int *exit_code){

}

void sched_nice(int niceval){

}

pid_t sched_getpid(){
	return getpid();
}

pid_t sched_getppid(){
	return getpid();
}

int sched_gettick(){
	return 0;
}

void sched_ps(){ // Sighandler

}

void sched_switch(){

}

void sched_tick(){ // Sighandler
	printf("Timer\n");
}
