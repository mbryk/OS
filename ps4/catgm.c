#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/signal.h>

int flog; char *lgbuf;

void int_handler(int sn){
	fprintf(stderr, "I just got a quit!");
	lgbuf = "LQUIT";
	exit(-1);
}

void pipe_handler(int sn){
	fprintf(stderr, "I just got a pipe");
	lgbuf = "LPIPE";
}

void childhandler(int sn){
	fprintf(stderr, "I just had a baby");
	lgbuf = "LPIPE";
}

int main(int argc, char **argv){
	int i, r, n, in_fd, status;
	pid_t pid, pid2;
	int b_size = 4096;

	char *grepargs[3]= {"grep", argv[1], NULL};
	char *pgargs[2]= {"pg", NULL};

	int fds_mgrep[2], fds_gpg[2];
	char *buf = malloc(b_size);
	if(buf==NULL){
		fprintf(stderr, "Error Malloc");
		exit(-1);
	}
	flog = open("lg.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
	for(i=2;i<argc;i++){
		pipe(fds_mgrep);
		pipe(fds_gpg);
		switch(pid = fork()){
			case -1: exit(-1);break;
			case 0:
				/* In Child to do pg! */
				/* Set up pipe read */

//				signal(SIGINT, interrupt_handler);
				dup2(fds_gpg[0], 0);
				close(fds_gpg[0]); close(fds_gpg[1]);
				close(fds_mgrep[0]); close(fds_mgrep[1]);
				execvp(pgargs[0],pgargs);
				break;
			default:
				break;
		}
		switch(pid2 = fork()){
			case -1: exit(-1); break;
			case 0:
				/* In Child to do grep! */
				/* Set up Pipe Read From Main and Pipe Write */
				signal(SIGPIPE, pipe_handler);
				dup2(fds_mgrep[0], 0);
				dup2(fds_gpg[1], 1);
				close(fds_gpg[0]); close(fds_gpg[1]);
				close(fds_mgrep[0]); close(fds_mgrep[1]);
				execvp("grep", grepargs);
				break;
			default:
				break;
		}
		close(fds_gpg[0]); close(fds_gpg[1]);
		close(fds_mgrep[0]);
		in_fd = open(argv[i], O_RDONLY, 0666);
		char *buf_tmp;
		while((r=read(in_fd, buf, b_size))>0){
			buf_tmp = buf;
			while((n=write(fds_mgrep[1], buf_tmp, r))!=r){
				if(n==-1){
					fprintf(stderr, "ERROR. %s", strerror(errno));
				}
				buf_tmp = buf_tmp+n;
				r-=n; 
			}
		}
		if(r==-1){
			exit(-1);
		}
		close(in_fd); 
		waitpid(pid,&status); 
		close(fds_mgrep[1]);
	}
	free(buf);
	return 0;
}
