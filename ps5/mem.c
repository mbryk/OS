/* Mark Bryk, OS, PS5 MEM.C */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>

char *filename = "testfile";
/* My Signals list is a bit different than yours. Platform specific order */
char *signals[31] = {"SIGHUP","SIGINT","SIGQUIT","SIGILL","SIGTRAP","SIGABRT","SIGBUS","SIGFPE","SIGKILL","SIGUSR1","SIGSEGV","SIGUSR2","SIGPIPE","SIGALRM","SIGTERM","SIGSTKFLT","SIGCHLD","SIGCONT","SIGSTOP","SIGTSTP","SIGTTIN","SIGTTOU","SIGURG","SIGXCPU","SIGXFSZ","SIGVTALRM","SIGPROF","SIGWINCH","SIGIO","SIGPWR","SIGSYS"};
jmp_buf int_jb;
void EC();

int create_file(int size, int flags){
	printf("Creating %d byte random file %s\n", size, filename);
	char command[100];
	sprintf(command, "dd if=/dev/urandom of=%s bs=%d count=1", filename, size);
	system(command);
	int fd = open(filename, flags);
	return fd;
}
static void sig_handlerA(int sn){
	printf("In response to question A: Signal %s is generated.\n", signals[sn-1]);
	unlink(filename);
	exit(0);
}
static void sig_handlerF(int sn){
	printf("Uh oh!\n");
	printf("In response to question F:\nAccessing first page results in no signal. The byte returned is a 0.\nAccessing second page results in a %s.\n", signals[sn-1]);
	/* This happens because of demand paging. */
	unlink(filename);
	exit(0);
}

int main(int argc, char **argv){
	char qc;
	int q;
	if(argc!=2){
		fprintf(stderr, "Error - Improper Input.\n");
		exit(-1);
	}
	int i, size, fd, offset;
	char *addr, *str, *answer;
	qc = argv[1][0];
	int flag = MAP_PRIVATE;
	switch(qc){
		case 'A': case 'a':
			for(i=1; i<32; i++){
				if(i==2||i==9||i==16||i==17||i==19) continue;
				if(signal(i, sig_handlerA)==SIG_ERR){
					fprintf(stderr,"Error- Setting Signal %d Handler: %s\n", i, strerror(errno));
				}
			}
			size = 4096;
			fd = create_file(size, O_RDWR);
			printf("About to MAP_SHARED with PROT_READ from fd %d\n", fd);
			addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
			close(fd);
			printf("About to write one byte to address %p\n", addr);
			*addr = 'A'; 
			printf("In response to question A: Nothing Happened!"); // Not gonna happen
			unlink(filename);
			exit(0);
			break;

		case 'B': case 'b':
			flag = MAP_SHARED;
		case 'C': case 'c':
			size = 4097;
			fd = create_file(size, O_RDWR);
			char *maps[2] = {"MAP_SHARED","MAP_PRIVATE"};
			printf("About to %s with read & write from fd %d\n", maps[flag-1], fd);
			addr = mmap(NULL, size, PROT_READ|PROT_WRITE, flag, fd, 0);
			printf("About to write 4 bytes to offset 30 from address %p\n", addr);
			offset = 30;
			str = "ABCD";
			for(i=0; i<4; i++) addr[offset+i] = str[i];
			lseek(fd, offset, SEEK_SET);
			char bufC[5];
			read(fd, bufC, 4);
			printf("Write to Memory: "); for(i=0;i<4;i++) printf("<%02X> ", str[i]);
			printf("\nRead from File: "); for(i=0;i<4;i++) printf("<%02X> ", bufC[i]);
			answer = strncmp(str,bufC,4)?"NO, the file is not updated with the write to memory":"YES, the update is immediately visible";
			printf("\nIn response to question %c: %s.\n", qc, answer);
			close(fd);
			unlink(filename);	
			break;

		case 'D': case 'd': case 'E': case 'e':
			size = 4097;
			fd = create_file(size, O_RDWR);
			int filesize = 4093;
			//	filesize = stat it!;
			printf("The size of the file is %d bytes\n", filesize);
			printf("About to MAP_SHARED with read & write from fd %d\n", fd);
			addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
			printf("About to write 5 bytes to offset %d\n", size);
			str = "ABCDE";
			for(i=0;i<5;i++) addr[size+i] = str[i];
			//	filesize = stat it!;
			printf("The new size of the file is %d bytes\n", filesize);
			printf("\nMemory dump starting at offset %d:\n",size);
			for(i=0;i<5;i++) printf("<%02X> ",addr[size+i]);
			lseek(fd, size, SEEK_SET);
			char bufD[20];
			read(fd, bufD, 5);
			printf("\nFile dump starting at offset %d:\n", size);
			for(i=0;i<5;i++) printf("<%02X> ",bufD[i]);
			answer = strncmp(str,bufD,5)?"NO, the file does not change":"YES, the file does change";
			printf("\nIn response to question D: %s.\n\n", answer); // No. It does not change.
			printf("About to expand file by 10 bytes and write 4 bytes to the end.\n");
			char *str2 = "FGHI";
			lseek(fd, size+10, SEEK_SET);
			write(fd, str2, 4);
			printf("Memory dump starting at offset %d:\n", size);
			for(i=0;i<14;i++) printf("<%02X> ",addr[size+i]);
			lseek(fd, size, SEEK_SET);
			read(fd, bufD, 14);
			printf("\nFile dump starting at offset %d:\n", size);
			for(i=0;i<14;i++) printf("<%02X> ",bufD[i]);
			answer = strncmp(bufD, str,5)?"NOT":"indeed";
			printf("\nIn response to question E: The data is %s visible in the file!\n", answer);
			close(fd);
			unlink(filename);
			break;

		case 'F': case 'f':
			for(i=1; i<32; i++){
				if(i==2||i==9||i==16||i==17||i==19) continue;
				if(signal(i, sig_handlerF)==SIG_ERR){
					fprintf(stderr,"Error- Setting Signal %d Handler: %s\n", i, strerror(errno));
				}
			}
			size = 12;
			fd = create_file(size, O_RDWR);
			printf("About to MAP_SHARED with PROT_READ from fd %d\n", fd);
			addr = mmap(NULL, 8192, PROT_READ, MAP_SHARED, fd, 0);
			close(fd);
			char c;
			offset = 30;
			printf("About to read one byte from first page mapped... ");
			c = addr[offset];
			printf("Nothing Happened!\n");
			printf("About to read one byte from second page mapped... ");	
			c = addr[4096+offset];
			printf("Nothing Happened?\n"); /* Not gonna happen */
			break;
		case 'G': case 'g': /* Extra Credit */
			EC();
			break;
		default:
			fprintf(stderr, "Error- Improper Input.\n");
			exit(-1);
			break;
	}
	return 0;
}
static void sig_handlerEC(int sn){
//	printf("HEY BABE %d", sn);
	longjmp(int_jb,1);
}
void EC(){
	int i, addr1; char c;
	signal(7, sig_handlerEC);
	signal(11, sig_handlerEC);
	char *addr;
	addr = mmap(NULL, 22, PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	for(i=2;;i++){
		if(setjmp(int_jb)!=0) {
			if(!addr1){
				addr1 = i;
				continue;
			} else	break;
		}
		c = addr[i];
	}
	printf("%d", i);
	printf("%d", addr1);
}
