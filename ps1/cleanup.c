#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv){
	int i;
	for(i=3;i++;i<7){
		close(i);
	}
	return 0;
}
