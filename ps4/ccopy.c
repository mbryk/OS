/* Mark Bryk, October 23rd 2013, OS PS4, CATGREPMORE */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

int i, bytes;
pid_t pidgrep, pidmore;

static void int_handler(int sn){
	fprintf(stderr, "\nComplete files Processed: %d\nBytes Processed: %d\n", i-2,bytes);
	exit(-1);
}

static void pipe_handler(int sn){
	if(kill(pidgrep, 9)==-1){
		fprintf(stderr, "Error- Killing grep process #%d on exit of pager\n", pidgrep, strerror(errno));
		exit(-1);
	}
}

int main(int argc, char **argv){
	int r, r_full, n, in_fd, stat1, stat2;
	int b_size = 4096;
	int fds_mgrep[2], fds_gmore[2];
	bytes = 0;
	char *grepargs[3]= {"grep", argv[1], NULL};
	char *moreargs[2]= {"more", NULL};
	char *buf = malloc(b_size);
	char *buf_tmp; /* This pointer will point to same location as buf, so don't need to malloc it more mem */
	if(buf==NULL || buf_tmp==NULL){
		perror("Error- Allocating Memory");
		exit(-1);
	}
	
	if(signal(SIGINT, int_handler)==SIG_ERR || signal(SIGPIPE, pipe_handler)==SIG_ERR){
		perror("Error- Setting Signal Handler");
		exit(-1);
	}
	for(i=2;i<argc;i++){
		if(pipe(fds_mgrep)==-1 || pipe(fds_gmore)==-1){
			perror("Error- Creating pipe");
			exit(-1);
		}
		switch(pidmore = fork()){
			case -1:
				perror("Error- Forking more child");
				exit(-1);
				break;
			case 0:
				if(dup2(fds_gmore[0], 0)==-1){
					perror("Error- Duping pipe to stdin of more child");
					exit(-1);
				}
				if(close(fds_gmore[0])==-1 || close(fds_gmore[1])==-1 || close(fds_mgrep[0])==-1 || close(fds_mgrep[1])==-1){
					perror("Error- Closing File Descriptor of Pipe");
					exit(-1);
				}
				if(execvp(moreargs[0],moreargs)==-1){
					perror("Error- Executing more process");
					exit(-1);
				}
				break;
			default:
				break;
		}
		switch(pidgrep = fork()){
			case -1:
				perror("Error- Forking grep child");
				exit(-1);
				break;
			case 0:
				if(signal(SIGPIPE, pipe_handler)==SIG_ERR){
					perror("Error- Setting Signal Handler");
					exit(-1);
				}
				if(dup2(fds_mgrep[0], 0)==-1 || dup2(fds_gmore[1], 1)==-1){
					perror("Error- Duping pipe to stdin of more child");
					exit(-1);
				}
				if(close(fds_gmore[0])==-1 || close(fds_gmore[1])==-1 || close(fds_mgrep[0])==-1 || close(fds_mgrep[1])==-1){
					perror("Error- Closing File Descriptor of Pipe");
					exit(-1);
				}
				if(execvp(grepargs[0],grepargs)==-1){
					perror("Error- Executing grep process");
					exit(-1);
				}
				break;
			default:
				break;
		}
		if(close(fds_gmore[0])==-1 || close(fds_gmore[1])==-1 || close(fds_mgrep[0])==-1){
			perror("Error- Closing File Descriptor of Pipe");
			exit(-1);
		}
		if((in_fd=open(argv[i], O_RDONLY, 0666))==-1){
			fprintf(stderr, "Error- Opening input file %s for reading: %s\n", argv[i], strerror(errno));
			exit(-1);
		}
		while((r=read(in_fd, buf, b_size))!=0){
			if(r==-1){
				fprintf(stderr, "Error- Reading from input file %s: %s\n", argv[i], strerror(errno));
			}
			buf_tmp = buf;
			r_full = r;
			while((n=write(fds_mgrep[1], buf_tmp, r))!=r && n!=-1){
				buf_tmp = buf_tmp+n;
				r-=n; 
			}
			if(n==-1) {
				if(errno==EPIPE) break;
				perror("Error- Writing into pipe");
				exit(-1);
			}
			bytes += r_full;
		}
		if(close(in_fd)==-1 || close(fds_mgrep[1])==-1){
			perror("Error- Closing File Descriptor of Pipe");
			exit(-1);
		}
		if(waitpid(pidgrep, &stat1, 0)==-1 || waitpid(pidmore,&stat2, 0)==-1){
			perror("Error- Returning from child process");
			exit(-1);
		}
	}
	free(buf);
	return 0;
}
