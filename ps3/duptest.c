#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

int main(){
	int pid, fd, status;
	char s[100];
	int redir = 1;
	int flags = O_CREAT | O_WRONLY | O_APPEND;
/*	flags = O_RDONLY;*/
	char *file = "file";
	switch(pid = fork()){
		case -1:
			fprintf(stderr, "error1", strerror(errno)); 
			exit(-1);
			break;
		case 0:
			fd = open(file, flags, 0666);
			close(redir);
			dup2(fd, redir);
			close(fd);
			fgets(s, 100, stdin);
			fprintf(stdout, "%s", s);
			fprintf(stderr, "HEY ERR!");
			fprintf(stdout, "HEY OUT!");
			exit(-1);

			break;			
		default:
			waitpid(pid, &status, 0);
			break;
	}
	return 0;
}
