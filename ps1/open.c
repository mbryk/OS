#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

int openfile(char **argv, int index, int flags){
	int fd;
	fd = open(argv[index], flags);
	return fd;
}


int readbuf(int fd, char *buf, int b_size){
	int n = read(fd, buf, b_size);
	return n;
}


void writebuf(int fd, char *buf, int b_size){
	int n = write(fd, buf, b_size);
	if(n != b_size){
		if(n <= 0){
			write(2, "Error Write\n", 12);
		}
		else {
			b_size -= n;
			writebuf(fd, buf, b_size);
		}
	}
}

void clearbuf(char *buf){


}

void closefile(int fd){
	close(fd);
}

int main(int argc, char **argv){
	int in_fd, out_fd, b_size, in_index, out_index, readsize;
	char *fname;
	int i=1;
	int j;
	
	if(argc<i) return 1;
	if(strcmp(argv[i],"-b") == 0){
		b_size = atoi(argv[++i]);
		++i;
	} else {
		b_size = 128;
	}

	int n = b_size;
	char *buf = malloc(b_size);

	if(argc<i) return 1;
	if(strcmp(argv[i],"-o") == 0){
	        out_index = ++i;
		out_fd = openfile(argv, out_index, O_CREAT | O_WRONLY);
		++i;
	} else {
		out_fd = 1; /* standard output */
	}

	in_index = i;
	for(j=in_index; j<argc; j++){
		in_fd = strcmp(argv[j],"-")?openfile(argv, in_index, O_RDONLY):0;
		while(n>0){
			clearbuf(buf);
			n = readbuf(in_fd, buf, b_size);
			writebuf(out_fd,buf, n);
		}
		closefile(in_fd);
	} 

	closefile(out_fd);
	free(buf);
	
	return 0;
}
