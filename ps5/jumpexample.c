#include <setjmp.h>
#include <stdio.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/types.h>

jmp_buf env;
static void handlera(int sn){
	printf("%d", sn);
	signal(SIGINT, SIG_DFL);
	longjmp(env, 1);
	printf("heya");
}
int main(){
	if(signal(SIGINT,handlera)==SIG_ERR)
		printf("Bagel button hosen shmear");
	if(setjmp(env)!=0)
	{
		printf("HEY");
	}
	for(;;);
/*
  int val;

  val=setjmp(env);

  printf ("val is %d\n",val);

  if (!val) longjmp(env, 1);

*/
	return 0;
}
