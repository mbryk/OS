#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

void redirection(char *file, int redir, int flags){
	int fd = open(file, flags, 0666);
	if(fd==-1){
		fprintf(stderr, "Error- Opening File %s for redirection to fd %d: %s", file, redir, strerror(errno));
		exit(-1);
	}
	if(dup2(fd, redir)==-1){
		fprintf(stderr, "Error- Copying File %s to new fd %d: %s", file, redir, strerror(errno));
		exit(-1);
	}
	close(fd);
}

int main(int argc, char **argv){
	int n, pid, status, i, j;
	int arg_in, dup_in, dup_out, dup_err, flags_out, flags_err;
	double rseconds;
	char *file_in, *file_out, *file_err, *arg, *buf;
	struct rusage ru;
	struct timeval begin, end;
	FILE *file;
	size_t len = 0;
	int arg_size = 5;
	char **args = calloc(arg_size, sizeof(char*));
	if(args==NULL){
		fprintf(stderr, "Error- Allocating Memory: %s\n", strerror(errno));
		exit(-1);
	}
	if(argc>1){
		/* r=read-only, e=Close on exec. This way, there are no possible open fd's when forking so I don't need to loop through all possible fd's */
		if((file = fopen(argv[1], "re"))==NULL){
			fprintf(stderr, "Error- Opening file %s for shell interpreting: %s", argv[1], strerror(errno));
			exit(-1);
		}
	} else file = stdin;

	while(1){
		dup_in = 0; dup_out = 0; dup_err = 0;
		if((n=getline(&buf, &len, file))==-1){
			fprintf(stderr,"End of File\n");break;
		}
		if(buf[n-1]=='\n')
			buf[n-1] = '\0';
		
		arg = strtok(buf, " ");
		if(arg==NULL||arg[0]=='#')
			continue;
		args[0] = arg;
		i=1;
		while((arg = strtok(NULL, " ")) != NULL){
			switch(arg[0]){
				case '<':
					dup_in = 1;
					arg++;		
					file_in = arg;
					break;
				case '>':
					dup_out = 1;
					arg++;
					flags_out = O_CREAT | O_WRONLY | O_TRUNC;
					if(arg[0]=='>'){
						flags_out = O_CREAT | O_WRONLY | O_APPEND;
						arg++;
					}
					file_out = arg;
					break;
				case '2':
					if(arg[1]=='>'){
						dup_err = 1;
						arg = arg+2;
						flags_err = O_CREAT | O_WRONLY | O_TRUNC; 
						if(arg[0]=='>'){
							flags_err = O_CREAT | O_WRONLY | O_APPEND;
							arg++;
						}
						file_err = arg;
						break;
					} /* Else, it is a regular argument and will fall into default */
				default:
					if(i==arg_size){
						arg_size *=5;
						args = realloc(args, arg_size*sizeof(char*));
						if(args==NULL){
							fprintf(stderr, "Error- Reallocating Memory: %s\n", strerror(errno));
							exit(-1);
						}
					}
					args[i] = arg;
					i++;
					break;
			}
		}
		args[i] = NULL;
		
		fprintf(stderr, "Executing command %s", args[0]);
		if(i!=1)
			fprintf(stderr, " with arguments ");
		for(j=1;args[j]!=NULL;j++)
			fprintf(stderr, "\"%s\" ", args[j]);
		fprintf(stderr, "\n");

		switch(pid = fork()){
			case -1:
				fprintf(stderr,"Error- Fork Failed: %s\n",strerror(errno));
				exit(-1);
				break;
			case 0:
				/* In Child */
				if(dup_in) redirection(file_in, 0, O_RDONLY);
				if(dup_out) redirection(file_out, 1, flags_out);
				if(dup_err) redirection(file_err, 2, flags_err);
				if((n = execvp(args[0], args))==-1){
					fprintf(stderr,"Error- Exec Failed: %s\n",strerror(errno));
					exit(-1);
				}
				break;
			default:
				/* In parent. ChildPID = pid */
				if(gettimeofday(&begin, NULL)==-1){
					fprintf(stderr, "Error- Recording time of child process %d on command %s: %s", pid, args[0], strerror(errno));
					exit(-1);
				};
				if(wait3(&status, 0, &ru)==-1){
					fprintf(stderr, "Error- Returning from child process %d on command %s: %s", pid, args[0], strerror(errno));
					exit(-1);
				}
				if(gettimeofday(&end, NULL)==-1){
					fprintf(stderr, "Error- Recording time of child process %d on command %s: %s", pid, args[0], strerror(errno));
					exit(-1);
				};
				break;
		}
		rseconds = difftime(end.tv_sec, begin.tv_sec) + difftime(end.tv_usec, begin.tv_usec)/1000000;
		/* I use both rusage and gettimeofday since real > user + system. 
		However, I think that part of the discrepancy comes from the time to perform the gettimeofday command!	*/
		if(WIFSIGNALED(status))
			fprintf(stderr, "Child [%d] terminated by signal %s", pid, strsignal(WTERMSIG(status)));
		if(WIFEXITED(status))
			fprintf(stderr, "Child [%d] returned with return code %d,\n", pid, WEXITSTATUS(status));
		fprintf(stderr, "consuming %.6f real seconds, ", rseconds);
		fprintf(stderr, "%ld.%.6d user, %ld.%.6d system\n\n", ru.ru_utime.tv_sec, ru.ru_utime.tv_usec, ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
	}
	fclose(file);
	free(args);
}
