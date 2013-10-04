#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void directory(char *dir);

void file_read(struct dirent *de, char *pathname){
		
		struct stat st;
		struct passwd *pwd;
		struct group *grp;
		struct tm *time;
/*		char *months[12] = ["Jan",  "Feb",  "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];*/
		char time_buf[17];
		
		stat(pathname, &st);

		pwd = getpwuid(st.st_uid);
		grp = getgrgid(st.st_gid);
		time = localtime(st.st_mtime);
		strftime(time_buf, 17, "%b %d %Y %H:%M", time);
/*		month = months[time->tm_mon];*/


		printf("%hd/%lu ", st.st_dev,de->d_ino);
		printf("%hu %hd ", st.st_mode, st.st_nlink);
		printf("%s %s ", pwd->pw_name, grp->gr_name);
		printf("%ld ",st.st_size);
		printf("%s", time_buf);
		printf("%s\n", pathname);
}

void directory(char *direct){
	DIR *dirp;
	struct dirent *de;

	if(!(dirp = opendir(direct))){
		printf("Can not open directory %s:%s\\n", direct, strerror(errno));
		return;
	}
	while(de = readdir(dirp)){
		if(strcmp(de->d_name,".") && strcmp(de->d_name,"..")){
		strcat(direct, "/");
		strcat(direct, de->d_name);
		if(de->d_type == DT_DIR){
			directory(direct);
		} else {
			file_read(de, direct);
		}
		}
	}
	closedir(dirp);
}

int main(int argc, char **argv){
	directory(argv[argc-1]);
	return 0;

}
