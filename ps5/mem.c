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

char *filename;
/* My Signals list is a bit different than yours. Platform specific order */
char *signals[31] = {"SIGHUP","SIGINT","SIGQUIT","SIGILL","SIGTRAP","SIGABRT","SIGBUS","SIGFPE","SIGKILL","SIGUSR1","SIGSEGV","SIGUSR2","SIGPIPE","SIGALRM","SIGTERM","SIGSTKFLT","SIGCHLD","SIGCONT","SIGSTOP","SIGTSTP","SIGTTIN","SIGTTOU","SIGURG","SIGXCPU","SIGXFSZ","SIGVTALRM","SIGPROF","SIGWINCH","SIGIO","SIGPWR","SIGSYS"};
void qA(); void qBC(int); void qDE(); void qF();

int main(int argc, char **argv){
	char qc;
	int q;
	if(argc!=2){
		fprintf(stderr, "Error - Improper Input.\n");
		exit(-1);
	}
	qc = argv[1][0];
	switch(qc){
		case 'A':
		case 'a':
			qA();
			break;
		case 'B':
		case 'b':
			qBC(MAP_SHARED);
			break;
		case 'C':
		case 'c':
			qBC(MAP_PRIVATE);
			break;
		case 'D':
		case 'd':
		case 'E':
		case 'e':
			qDE();
			break;
		case 'F':
		case 'f':
			qF();
			break;
		default:
			fprintf(stderr, "Error- Improper Input.\n");
			exit(-1);
			break;
	}
	return 0;
}

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
	exit(-1);
}

void qA(){
	int i;
	for(i=1; i<32; i++){
		if(i==2||i==9||i==16||i==17||i==19) continue;
		if(signal(i, sig_handlerA)==SIG_ERR){
			fprintf(stderr,"Error- Setting Signal %d Handler: %s\n", i, strerror(errno));
		}
	}
	int size = 4096;
	filename = "testfileA";
	int fd = create_file(size, O_RDWR);

	char *addr;
	printf("About to MAP_SHARED with PROT_READ from fd %d\n", fd);
	addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	printf("About to write one byte to address %p\n", addr);
	*addr = 'A'; 

	/* No need to unmap or reset signal handlers, because the signal handler will kill the process and the virtual address space will be gone. */
}

void qBC(int flag){
	int i;
	int size = 4096;
	filename = "testfileBC";
	int fd = create_file(size, O_RDWR);
	char *addr;
	char *maps[2] = {"MAP_SHARED","MAP_PRIVATE"};
	printf("About to %s with read & write from fd %d\n", maps[flag-1], fd);
	addr = mmap(NULL, size, PROT_READ|PROT_WRITE, flag, fd, 0);
	
	printf("About to write 4 bytes to offset 30 from address %p\n", addr);
	int offset = 30;
	char *str = "ABCD";
	for(i=0; i<4; i++)
		addr[offset+i] = str[i];
	lseek(fd, offset, SEEK_SET);
	char buf[5];
	read(fd, buf, 4);
	buf[4] = '\0';
	printf("Write to Memory: "); for(i=0;i<4;i++) printf("<%02X> ", str[i]);
	printf("\nRead from File: "); for(i=0;i<4;i++) printf("<%02X> ", buf[i]);
	if(flag==1) printf("\nIn response to question B: YES, the update is immediately visible.\n");
	else printf("\nIn response to question C: NO, the file is not updated with the write to memory.\n");
	close(fd);
	unlink(filename);	
}

void qDE(){
	int i;
	int size = 4093;
	filename = "testfileDE";
	int fd = create_file(size, O_RDWR);
	int filesize = 4093;
//	filesize = stat it!;
	printf("The size of the file is %d bytes\n", filesize);
	char *addr;
	printf("About to MAP_SHARED with read & write from fd %d\n", fd);
	addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	printf("About to write 5 bytes to offset %d\n", size);
	char *str = "ABCDE";
	for(i=0;i<5;i++)
		addr[size+i] = str[i];
//	filesize = stat it!;
	printf("The new size of the file is %d bytes\n", filesize);

	printf("\nMemory dump starting at offset %d:\n",size);
	for(i=0;i<5;i++) printf("<%02X> ",addr[size+i]);
	
	lseek(fd, size, SEEK_SET);
	char buf[20];
	read(fd, buf, 5);
	buf[5] = '\0';
	printf("File dump starting at offset %d:\n", size);
	for(i=0;i<5;i++) printf("<%02X> ",buf[i]);
	
	printf("\nIn response to question D: NO, the file does not change.\n");
	
	printf("About to expand file by 10 bytes and write 4 bytes to the end.\n");
	str = "FGHI";
	lseek(fd, size+10, SEEK_SET);
	write(fd, str, 4);
	printf("Memory dump starting at offset %d:\n", size);
	for(i=0;i<14;i++) printf("<%02X> ",addr[size+i]);
	lseek(fd, size, SEEK_SET);
	read(fd, buf, 14);
	printf("\nFile dump starting at offset %d:\n", size);
	for(i=0;i<14;i++) printf("<%02X> ",buf[i]);
	printf("\nIn response to question E: The data is indeed visible in the file!\n");
	close(fd);
	unlink(filename);
}

static void sig_handlerF(int sn){
	printf("Uh oh!\n");
	printf("In response to question F:\nAccessing first page results in no signal. The byte returned is a 0.\nAccessing second page results in a %s.\n", signals[sn-1]);
	/* This happens because of demand paging. */
	unlink(filename);
	exit(0);
}

void qF(){
	int i;
	for(i=1; i<32; i++){
		if(i==2||i==9||i==16||i==17||i==19) continue;
		if(signal(i, sig_handlerF)==SIG_ERR){
			fprintf(stderr,"Error- Setting Signal %d Handler: %s\n", i, strerror(errno));
		}
	}
	int size = 12;
	filename = "testfileF";
	int fd = create_file(size, O_RDWR);
	char *addr;
	printf("About to MAP_SHARED with PROT_READ from fd %d\n", fd);
	addr = mmap(NULL, 8192, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	char c;
	int offset = 30;
	printf("About to read one byte from first page mapped... ");
	c = addr[offset];
	printf("Nothing Happened!\n");
	printf("About to read one byte from second page mapped... ");
	c = addr[4096+offset];
	printf("Nothing Happened?\n"); /* Not gonna happen */
}
