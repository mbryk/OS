#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv){
	int i, r, n, in_fd, status;
	pid_t pid, pid2;
	int b_size = 4096;

	char *grepargs[3]= {"grep", argv[1], NULL};
	char *pgargs[2] = {"pg", NULL};

	int fds_mgrep[2], fds_gpg[2];
	char *buf = malloc(b_size);
	if(buf==NULL){
		fprintf(stderr, "Error Malloc");
		exit(-1);
	}
	int va,vb;
	for(i=2;i<argc;i++){
		va = pipe(fds_mgrep);
		vb = pipe(fds_gpg);
		switch(pid = fork()){
			case -1: exit(-1);break;
			case 0:
				/* In Child to do pg! */
				/* Set up pipe read */
				dup2(fds_gpg[0], 0);
				close(fds_gpg[0]); close(fds_gpg[1]);
				close(fds_mgrep[0]); close(fds_mgrep[1]);
				execvp("pg", pgargs);
				break;
			default:
				break;
		}
		switch(pid2 = fork()){
			case -1: exit(-1); break;
			case 0:
				/* In Child to do grep! */
				/* Set up Pipe Read From Main and Pipe Write */
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
		while((r=read(in_fd, buf, b_size))>0){
			while((n=write(fds_mgrep[1], buf, r))!=r){
				if(n==-1){
					exit(-1);
				}
				buf = buf+n;
				r-=n; 
			}
		}
		if(r==-1){
			exit(-1);
		}
		close(fds_mgrep[1]);
		waitpid(pid2, &status); 
		close(fds_gpg[1]);
		waitpid(pid, &status);
		close(in_fd); 
	}
	free(buf);
	return 0;
}
