#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int readbuf(int fd, char *buf, int b_size){
	int n;
	if((n=read(fd, buf, b_size))==-1){
		fprintf(stderr, "Error Reading from File: %s\n", strerror(errno));
		exit(-1);
	}
	return n;
}

void writebuf(char *buf, int writesize){
	int n = write(fd, buf, writesize);
	if(n != writesize){
		if(n < 1){
			/* It is possible that if n==0, errno is not set. */
			if(errno) fprintf(stderr, "Error Writing to File: %s\n", strerror(errno));
			else fprintf(stderr, "Error Writing to File\n");
			exit(-1);
		} else {
			/* For Partial Writes, set new writesize to remaining characters, and recursively call function, starting buf at next unwritten char */
			writebuf(fd, buf+n, writesize-n);
		}
	}
}

int main(int argc, char **argv){
	int i;

	char *grepargs[5], *pgargs[1];
	grepargs[0] = "grep";
	grepargs[1] = argv[1];
	pgargs[0] = "more";

	int fds[2];
	for(i=2;i<argc;i++){
		pipe(fds);
		switch(pid = fork()){
			case -1: exit(-1);break;
			case 0:
				/* In Child to do grep! */
				/* Set up Pipe */
				grepargs[2] = argv[i];
				grepargs[3] = NULL;
				dup2(fds[0], 0);
				close(fds[0]);
				execvp("grep", grepargs);
				break;
			default:
				break;
		}
		switch(pid2 = fork()){
			case -1: exit(-1); break;
			case 0:
				/* In Child to do pipe */
				/* Set up pipe */
				dup2(fds[1], 1);
				close(fds[1]);
				execvp("pg", pgargs);
				break;
			default:
				break;
		}

	}
	return 0;
}
