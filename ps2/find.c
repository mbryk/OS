#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

int main(int argc, char **argv){
	

DIR *dirp = NULL;
struct dirent *dp = NULL;

dirp = opendir(argv[argc-1]);
if(dirp==NULL)
	exit(1);
while((dp = readdir(dirp)) != NULL) {
	printf("%s\n", dp->d_name);
}
closedir(dirp);
return 0;

}
