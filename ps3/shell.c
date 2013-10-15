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

void redirection(char *file, int redir, int flags){
	int fd = open(file, flags, 0666);
	close(redir);
	dup2(fd, redir);
	close(fd);
}

void closeFds(){
	int fdLimit,i;
	if((fdLimit=getdtablesize())==-1){
		fprintf(stderr, "Error getting max possible file descriptor: %s", strerror(errno));
		exit(-1);
	}
	for(i=3; i<fdLimit; i++){
		close(i);
	}
}

int main(){
	int n, pid, status, i, j;
	int dup_in, dup_out, dup_err, flags_out, flags_err;
	double rseconds;
	char *file_in, *file_out, *file_err, *arg;
	struct rusage ru;
	struct timeval begin, end;

	int b_size = 1024;
	char *buf = malloc(b_size);
	char *nargv[10];
	while(1){
		dup_in = 0; dup_out = 0; dup_err = 0;
		fprintf(stderr, "--->");
		if((n=read(0, buf, b_size))==-1){
			fprintf(stderr, "Error - Reading from Shell failed: %s", strerror(errno));
			exit(-1);
		}

		buf[n-1] = '\0'; /* To get rid of new line */
		arg = strtok(buf, " ");
		
		if(arg==NULL||arg[0]=='#')
			continue;
		nargv[0] = arg;
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
					dup_err = 1;
					arg = arg+2;
					flags_err = O_CREAT | O_WRONLY | O_TRUNC; 
					if(arg[0]=='>'){
						flags_err = O_CREAT | O_WRONLY | O_APPEND;
						arg++;
					}
					file_err = arg;					
					break;
				default:
					nargv[i] = arg;
					i++;
					break;
			}
		}
		nargv[i] = NULL;
		
		fprintf(stderr, "Executing command %s", nargv[0]);
		if(i!=1)
			fprintf(stderr, " with arguments ");
		for(j=1;nargv[j]!=NULL;j++){
			fprintf(stderr, "\"%s\" ", nargv[j]);
		}
		fprintf(stderr, "\n\n");

		switch(pid = fork()){
			case -1:
				fprintf(stderr, "Error. Fork Failed: %s\n", strerror(errno));
				exit(-1);
				break;
			case 0:
				/* In Child */
				closeFds();
				if(dup_in) redirection(file_in, 0, O_RDONLY);
				if(dup_out) redirection(file_out, 1, flags_out);
				if(dup_err) redirection(file_err, 2, flags_err);

				if((n = execvp(nargv[0], nargv))==-1){
					fprintf(stderr, "Error. Exec Failed: %s\n", strerror(errno));
					exit(-1);
				}
				break;
			default:
				/* In parent. ChildPID = pid */
				gettimeofday(&begin, NULL);
				if(wait3(&status, 0, &ru)==-1){
					fprintf(stderr, "Error returning from child process on command %s: %s", nargv[0], strerror(errno));
					exit(-1);
				}
				gettimeofday(&end, NULL);
				break;
		}
		rseconds = difftime(end.tv_sec, begin.tv_sec) + difftime(end.tv_usec, begin.tv_usec)/1000000;
		/* I use both rusage and gettimeofday since real > user + system. 
		However, I think that part of the discrepancy comes from the time to perform the gettimeofday command!	*/
		fprintf(stderr, "\nCommand returned with return code %d,\n", status);
		fprintf(stderr, "consuming %.6f real seconds, ", rseconds);
		fprintf(stderr, "%ld.%.6d user, %ld.%.6d system\n\n", ru.ru_utime.tv_sec, ru.ru_utime.tv_usec, ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
	}
}
