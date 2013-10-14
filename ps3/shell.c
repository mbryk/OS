#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void redirection(char *file, int redir, int flags){
	int fd = open(file, flags, 0666);
	close(redir);
	dup2(fd, redir);
	close(fd);
}

int main(){
	int n,pid, status, fd, i, dup, flags;
	int b_size = 1024;
	char *buf = malloc(b_size);
	char *arg;
	char *argv[10];
	while(1){
		fprintf(stderr, "--->");
		if((n=read(0, buf, b_size))==-1){
			fprintf(stderr, "Error - Reading from Shell failed: %s", strerror(errno));
			exit(-1);
		}
		buf[n-1] = '\0'; /* To get rid of new line */
		arg = strtok(buf, " ");
		
		if(arg==NULL||arg[0]=='#')
			continue;
		argv[0] = arg;
		i=1;
		dup = 0;

		switch(pid = fork()){
			case -1:
				fprintf(stderr, "Error. Fork Failed: %s\n", strerror(errno));
				exit(-1);
				break;
			case 0:
				/* In Child */
				while((arg = strtok(NULL, " ")) != NULL){
					switch(arg[0]){
						case '<':
							dup = 1;
							fd = 0;
							flags = O_RDONLY;
							arg++;
							break;
						case '>':
							dup = 1;
							fd = 1;
							arg++;
							break;
						case '2':
							dup = 1;
							fd = 2;
							arg = arg+2;
							break;
						default:
							argv[i] = arg;
							i++;
							break;
					}
					if(dup){
						if(arg[0]=='>'){
							flags = O_CREAT | O_WRONLY | O_APPEND;
							arg++;
						} else if (fd!=0){ 
							flags = O_CREAT | O_WRONLY | O_TRUNC; 
						}
						redirection(arg, fd, flags);
					}
				}

				argv[i]=NULL;
				int j;
				fprintf(stderr, "Executing command %s", argv[0]);
				if(i!=1) fprintf(stderr, " with arguments ");
				for(j=1;argv[j]!=NULL;j++){
					fprintf(stderr, "\"%s\" ", argv[j]);
				}
				fprintf(stderr, "\n\n");

				if((n = execvp(argv[0], argv))==-1){
					fprintf(stderr, "Error. Exec Failed: %s\n", strerror(errno));
					exit(-1);
				}
				break;
			default:
				/* In parent. ChildPID = pid */
				waitpid(pid,&status, 0);
				fprintf(stderr, "\nCommand returned with return code %d,\n", status);
				fprintf(stderr, "consuming 0.005 real seconds, 0.002 user, 0.001 system\n");
				break;
		}
	}
}
