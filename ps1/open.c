#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int openfile(char **argv, int index, int flags){
	int fd;
	fd = open(argv[index], flags, 0777);
	if(fd == -1){
		fprintf(stderr, "Error opening file %s: %s\n", argv[index], strerror(errno));
		exit(-1);
	}
	return fd;
}


int readbuf(int fd, char *buf, int b_size){
	int n = read(fd, buf, b_size);
	if(n==-1){
		fprintf(stderr, "Error Reading from File: %s\n", strerror(errno));
		exit(-1);
	}
	return n;
}


void writebuf(int fd, char *buf, int writesize){
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

void closefile(int fd){
	if(fd>2){
		int n =	close(fd);
		if(n == -1){
			fprintf(stderr, "Error Closing File: %s\n", strerror(errno));
			exit(-1);
		}
	}
}

int main(int argc, char **argv){
	int in_fd, out_fd, in_index, out_index, b_size;
	int i, n;

	i=1;
	b_size = 4096; /* DEFAULT Buffer Size */
	out_fd = 1; /* DEFAULT Output is stdout=1 */
	while(argc>i){
		if(strcmp(argv[i],"-b") == 0){
			if(argc==++i){
				fprintf(stderr, "Error Parsing Arguments.\nNo buffer size is specified.\nOmit -b for default buffer size of 4096.\n");
				exit(-1);
			}
			b_size = atoi(argv[i]);
			if(b_size<=0){
				fprintf(stderr, "Error Setting Buffer Size.\n%d is not a valid buffer size.\nOmit -b for default buffer size of 4096.\n", b_size);
				exit(-1);
			}
			++i;
		}
		if(strcmp(argv[i],"-o") == 0){
		        out_index = ++i;
			if(argc==out_index){
				fprintf(stderr, "Error Parsing Arguments.\nNo outfile is specified.\nOmit -o to use standard output.\n");
				exit(-1);
			}
			out_fd = openfile(argv, out_index, O_CREAT | O_WRONLY | O_TRUNC);
			++i;
			continue;
		}
		break;
	}
	char *buf = malloc(b_size);
	in_index = i; /* Once done parsing the optional arguments, the next index is the first infile */

	do{
		n = 1; /* reset n to above zero for each new infile */

		/* This assigns input to stdin for when "-" is specified or no input file is given */
		in_fd = ((argc>in_index)&&strcmp(argv[in_index],"-"))?openfile(argv, in_index, O_RDONLY):0;
		while(n>0){
			n = readbuf(in_fd, buf, b_size);
			writebuf(out_fd,buf, n); /* Only write the amount that was read to buf */
		}
		closefile(in_fd);
		in_index++;
	} while(in_index<argc);

	closefile(out_fd);
	free(buf);
	
	return 0;
}
