#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(){
	int n;
	int b_size = 1024;
	char *buf = malloc(b_size);
	char *args;
	char *argv[];
	while(n=read(0, buf, b_size)){
		buf[n] = '\0';
		command = strtok(buf, " ");
		
		if(!strcmp(args, "#"))
			continue;
		printf("Executing command %s", command);
		argv[0] = command;
		int i=1;
		fork;
		while(args != NULL){
			args = strtok(NULL, " ");
			if(/* is redirection */){
				dupstuff;
			} else {
				argv[i]=args;
				i++;
			}
		}
		exec(shell, argv, envp);
	}
}
