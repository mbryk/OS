#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

static void handler(int sn){
	printf("handled\n");		
}

int main(){
	int p;
	sigset_t new_mask;
	sigfillset(&new_mask);
	sigdelset(&new_mask,SIGUSR1);
	int s;
	int secs = 2;
	time_t retTime;
	switch(p=fork()){
		case -1: perror("Error ");return -1;break;
		case 0: // In child
			signal(SIGUSR1, handler);
			printf("in child\n");
			sigsuspend(&new_mask);
			retTime = time(0) + secs;
			while(time(0)<retTime);
			printf("out of suspend\n");
			break;
		default: //In parent
			printf("in parent\n");
			retTime = time(0) + secs;
			while (time(0) < retTime);
			printf("sending signal\n");
			kill(p,SIGUSR1);
			printf("sent\n");
			waitpid(p,&s,0);
			break;
	}
	return 0;
}
