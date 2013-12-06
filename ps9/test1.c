#include "sched.h"
#include <time.h>

void a(){

}

int main(){
	sched_init(a);
	time_t t = time(0);
	while(time(0)-t<5);
	return 0;

}