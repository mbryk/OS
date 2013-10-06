#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

int opts;
int mdisplay=0;
int udisplay=-1;

static char filetypeletter(int mode)
{
    char    c;
    if (S_ISREG(mode))
        c = '-';
    else if (S_ISDIR(mode))
        c = 'd';
    else if (S_ISBLK(mode))
        c = 'b';
    else if (S_ISCHR(mode))
        c = 'c';
#ifdef S_ISFIFO
    else if (S_ISFIFO(mode))
        c = 'p';
#endif  /* S_ISFIFO */
#ifdef S_ISLNK
    else if (S_ISLNK(mode))
        c = 'l';
#endif  /* S_ISLNK */
#ifdef S_ISSOCK
    else if (S_ISSOCK(mode))
        c = 's';
#endif  /* S_ISSOCK */
#ifdef S_ISDOOR
    /* Solaris 2.6, etc. */
    else if (S_ISDOOR(mode))
        c = 'D';
#endif  /* S_ISDOOR */
    else
    {
        c = '-';
    }
    return(c);
}

int check_file(char *pathname){
	if(opts){
		struct stat st;
		lstat(pathname, &st); 
		if((udisplay>=0) && (udisplay!=st.st_uid))
			return 0;
		if(mdisplay){
			time_t tnow;
			time(&tnow);
			double seconds = difftime(tnow, st.st_mtime);
			return mdisplay>0?(mdisplay<seconds):(mdisplay+seconds<0);
		}
	}
	return 1;
}

void print_info(char *pathname){
		struct stat st;
		struct passwd *pwd;
		struct group *grp;
		struct tm *time;
		char time_buf[17];

		lstat(pathname, &st);

		pwd = getpwuid(st.st_uid);
		grp = getgrgid(st.st_gid);
		time_t t = st.st_mtime; 
		time = localtime(&t);
		strftime(time_buf, 18, "%b %d %Y %H:%M", time);
		
		char mode[11];
		static char *rwx[] = {"---", "--x", "-w-", "-wx","r--", "r-x", "rw-", "rwx"};
		mode[0] = filetypeletter(st.st_mode);
		strcpy(&mode[1], rwx[(st.st_mode >> 6)& 7]);
		strcpy(&mode[4], rwx[(st.st_mode >> 3)& 7]);
		strcpy(&mode[7], rwx[(st.st_mode & 7)]);
		mode[10] = '\0';

		if(S_ISLNK(st.st_mode)){
			char target[1024];
			int length = readlink(pathname, target, 1024);
			if(length<0){
				fprintf(stderr, "uhoh %s", strerror(errno));
				exit(-1);
			}
			target[length] = '\0';
			strcat(pathname, " -> ");
			strcat(pathname, realpath(target, NULL));
		}
		
		printf("%hd/%lu ", st.st_dev,st.st_ino);
		printf("%s %hd ", mode, st.st_nlink);
		printf("%s %s\t", pwd->pw_name, grp->gr_name);
		printf("%ld\t",st.st_size);
		printf("%s ", time_buf);
		printf("%s\n", pathname);

}

void directory(char *direct){
	DIR *dirp;
	struct dirent *de;

	if((dirp = opendir(direct))==NULL){
		printf("Can not open directory %s:%s\n", direct, strerror(errno));
		return;
	}

	while(de = readdir(dirp)){
		char pathname[1024];
		strcpy(pathname, direct);
		strcat(pathname, "/");
		strcat(pathname, de->d_name);
		
		/* I couldn't include this in check_file, since even if check_file returns 0, we still need to further check that directory... */
		if(strcmp(de->d_name,".") && strcmp(de->d_name,"..")){
			if(check_file(pathname))
				print_info(pathname);			
			if(de->d_type == DT_DIR)
				directory(pathname);
		}
	}

	if(closedir(dirp)<0){
		fprintf(stderr, "%s\n", strerror(errno));
		exit(-1);
	}
}

int main(int argc, char **argv){
	struct passwd *pwd;
	int c = 0;
	while ((c = getopt (argc, argv, "u:m:")) != -1){
		opts = 1;
		switch (c){
			case 'u':
				if((udisplay = atoi(optarg)) == 0){
					pwd = getpwnam(optarg);
					udisplay = pwd->pw_uid;
				}		
				break;
			case 'm':
				mdisplay = atoi(optarg);
				break;
		}
	}
	directory(realpath(argv[argc-1], NULL));
	return 0;
}

