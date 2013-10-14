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
	int fd, i, n;

	char *buf = malloc(4096);
	char *pattern = argv[1];

	for(i=2;i<argc;i++){
		if((fd = open(argv[i], O_RDONLY, 0666))==-1){
			fprintf(stderr, "Uhoh open");
			exit(-1);
		}

		while((n = readbuf(in_fd, buf, b_size))>0){
			writebuf(buf, n);
		}

		if(close(in_fd)==-1){
			fprintf(stderr, "Uhoh");
			exit(-1);
		}
	}
	free(buf);
	return 0;
}
