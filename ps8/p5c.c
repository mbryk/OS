/* Mark Bryk OS PS8 P5C.C */
#include <stdio.h>
#include <time.h>

int main(){
	int i;
	int t = 100000000; /* 100 Million Iterations */
	struct timespec begin,end;
	if(clock_gettime(CLOCK_REALTIME,&begin)==-1){ perror("Error Recording Begin Time of Loop"); return -1;}

	for(i=0;i<t;i++){
		getuid();
	}
	
	if(clock_gettime(CLOCK_REALTIME,&end)==-1){ perror("Error Recording End Time of Loop"); return -1;}
        long secs = end.tv_sec-begin.tv_sec;
        long nsecs = (end.tv_nsec-begin.tv_nsec);
        long total = (secs>0)?(secs*1000000000+nsecs):nsecs;
        double each = ((double)total) / t;
        printf("Execution Time = \t%ld ns\n",total);
        printf("Each Iteration = \t%f ns\n",each);
        return 0;

}
