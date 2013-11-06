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

char *filename = "testfile";
int create_file(int size, int flags){
	printf("Creating %d byte random file %s\n", size, filename);
	char command[100];
	sprintf(command, "dd if=/dev/urandom of=%s bs=%d count=1", filename, size);
	if(system(command)==-1){
		perror("Error creating random test file");
		exit(-1);
	}
	int fd;
	if((fd=open(filename, flags))==-1){
		perror("Error opening random test file");
		exit(-1);
	}
	return fd;
}

static void sig_handlerA(int sn){
	printf("In response to question A: Signal #%d is generated: %s\n",sn,strsignal(sn));
	unlink(filename);
	exit(0);
}
static void sig_handlerF(int sn){
	printf("Uh oh!\n");
	printf("In response to question F:\nAccessing first page results in no signal. The byte returned is a 0.\nAccessing second page results in signal #%d: %s.\n", sn, strsignal(sn));
	/* This happens because of demand paging. You make a memory region with mmap of two pages. However, when copying the data from the file, you only ask for one page, which then goes into the page table. So, when accessing the first page, regardless if the offset is past the end of the file, the PTE is already there. However, when accessing the second page, even though it's in a memory region, it doesn't correspond to a physical page, which sends a bus error. */
	unlink(filename);
	exit(0);
}

int main(int argc, char **argv){
	if(argc!=2 || strlen(argv[1])!=1){
		fprintf(stderr, "Error - Improper Input.\nOnly one input argument is allowed. Must be a letter from A-F.\n");
		exit(-1);
	}
	int i, size, fd, offset;
	char *addr, *str, *answer;
	struct stat st;
	int flag = MAP_PRIVATE;
	char qc = argv[1][0];
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
			if((addr=mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0))==MAP_FAILED){
				perror("Error Mapping File to Memory");
				exit(-1);
			}
			close(fd);
			printf("About to write one byte to address %p\n", addr);
			*addr = 'A'; 
			printf("In response to question A: Nothing Happened!"); // Not gonna happen
			unlink(filename);
			break;

		case 'B': case 'b':
			flag = MAP_SHARED;
		case 'C': case 'c':
			size = 4097;
			fd = create_file(size, O_RDWR);
			char *maps[2] = {"MAP_SHARED","MAP_PRIVATE"};
			printf("About to %s with read & write from fd %d\n", maps[flag-1], fd);
			if((addr=mmap(NULL, size, PROT_READ|PROT_WRITE, flag, fd, 0))==MAP_FAILED){
				perror("Error Mapping File to Memory");
				exit(-1);
			}
			printf("About to write 4 bytes to offset 30 from address %p\n", addr);
			offset = 30;
			str = "ABCD";
			for(i=0; i<4; i++) addr[offset+i] = str[i];
			if(lseek(fd, offset, SEEK_SET)==-1){
				perror("Error seeking through file");
				exit(-1);
			}
			char bufC[5];
			if(read(fd, bufC, 4)==-1){
				perror("Error Reading from file");
				exit(-1);
			}
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
			if(fstat(fd, &st)==-1){
				perror("Error getting info on testfile");
				exit(-1);
			}
			printf("The size of the file is %ld bytes\n", st.st_size);
			printf("About to MAP_SHARED with read & write from fd %d\n", fd);
			if((addr=mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))==MAP_FAILED){
				perror("Error Mapping File to Memory");
				exit(-1);
			}
			printf("About to write 5 bytes to offset %d\n", size);
			str = "ABCDE";
			for(i=0;i<5;i++) addr[size+i] = str[i];
			if(fstat(fd, &st)==-1){
				perror("Error getting info on testfile");
				exit(-1);
			}
			printf("The new size of the file is %d bytes\n", st.st_size);
			printf("\nMemory dump starting at offset %d:\n",size);
			for(i=0;i<5;i++) printf("<%02X> ",addr[size+i]);
			if(lseek(fd, size, SEEK_SET)==-1){
				perror("Error seeking through file");
				exit(-1);
			}
			char bufD[20];
			if(read(fd, bufD, 5)==-1){
				perror("Error reading from file");
				exit(-1);
			}
			printf("\nFile dump starting at offset %d:\n", size);
			for(i=0;i<5;i++) printf("<%02X> ",bufD[i]);
			answer = strncmp(str,bufD,5)?"NO, the file does not change":"YES, the file does change";
			printf("\nIn response to question D: %s.\n\n", answer); // No. It does not change.
			printf("About to expand file by 10 bytes and write 4 bytes to the end.\n");
			char *str2 = "FGHI";
			if(lseek(fd, size+10, SEEK_SET)==-1){
				perror("Error seeking through file");
				exit(-1);
			}
			if(write(fd, str2, 4)==-1){
				perror("Error expanding and writing to file");
				exit(-1);
			}
			printf("Memory dump starting at offset %d:\n", size);
			for(i=0;i<14;i++) printf("<%02X> ",addr[size+i]);
			if(lseek(fd, size, SEEK_SET)==-1){
				perror("Error seeking through file");
				exit(-1);
			}
			if(read(fd, bufD, 14)==-1){
				perror("Error reading from file");
				exit(-1);
			}
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
			if((addr=mmap(NULL, 8192, PROT_READ, MAP_SHARED, fd, 0))==MAP_FAILED){
				perror("Error Mapping file to memory");
				exit(-1);
			}
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
		default:
			fprintf(stderr, "Error - Improper Input.\nOnly one input argument is allowed. Must be a letter from A-F.\n");
			exit(-1);
			break;
	}
	return 0;
}
