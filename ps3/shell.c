#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(){
	int n,pid, status;
	int b_size = 1024;
	char *buf = malloc(b_size);
	char *arg;
	char *argv[10];

	while(n=read(0, buf, b_size)){
		if(n==-1){
		}
		buf[n-1] = '\0'; /* To get rid of new line */
		arg = strtok(buf, " ");
		
		if(buf=='\0'||arg[0]=='#')
			continue;
		printf("Executing command %s\n", arg);
		argv[0] = arg;
		int i=1;

		switch(pid = fork()){
			case -1:
				fprintf(stderr, "Error. Fork Failed: %s\n", strerror(errno));
				exit(-1);
				break;
			case 0:
				/* In Child */
				while((arg = strtok(NULL, " ")) != NULL){
					switch(arg[0]){
						case '>':
							break;
						case '<':
							break;
						case '2':
							break;
						default:
							argv[i] = arg;
							i++;
							break;
					}
				}

				argv[i]=NULL;

				if((n = execvp(argv[0], argv))==-1){
					fprintf(stderr, "Error. Exec Failed: %s\n", strerror(errno));
					exit(-1);
				}
				break;
			default:
				/* In parent. ChildPID = pid */
				waitpid(pid,&status, 0);
				printf("Back from child %d\n", pid);
				break;
		}
	}
}
