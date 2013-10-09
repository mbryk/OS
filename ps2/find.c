/* Mark Bryk, ECE357, Problem Set 2 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

int opts;
time_t tnow;
int mdisplay=0;
int udisplay=-1;
dev_t vol_start=0;
char *target_link=NULL;

static char filetypeletter(int mode)
{
    char    c;
    if (S_ISREG(mode)) c = '-';
    else if (S_ISDIR(mode)) c = 'd';
    else if (S_ISBLK(mode)) c = 'b';
    else if (S_ISCHR(mode)) c = 'c';
#ifdef S_ISFIFO
    else if (S_ISFIFO(mode)) c = 'p';
#endif  /* S_ISFIFO */
#ifdef S_ISLNK
    else if (S_ISLNK(mode)) c = 'l';
#endif  /* S_ISLNK */
#ifdef S_ISSOCK
    else if (S_ISSOCK(mode)) c = 's';
#endif  /* S_ISSOCK */
#ifdef S_ISDOOR
    /* Solaris 2.6, etc. */
    else if (S_ISDOOR(mode)) c = 'D';
#endif  /* S_ISDOOR */
    else	c = '-';
    return(c);
}

int check_file(char *pathname, char *filename){
	/* This function returns a 0 if the file is OK to print. Returns a 1 if it is not OK to print.
	Returns a 2 if it should not be stepped into, if it's a directory (a ".","..", or new volume). */
	
	if(!(strcmp(filename,".") && strcmp(filename,".."))) return 2;
	if(opts){ /*opts is set if there are options, in order to save an unnecessary lstat system call */
		struct stat st;
		if(lstat(pathname, &st)<0){
			fprintf(stderr, "Error getting information on file %s: %s\n", pathname, strerror(errno));
			exit(-1);
		} 
		if(vol_start){
			if(st.st_dev != vol_start){
				fprintf(stderr, "note: not crossing mount point at %s\n", pathname);
				return 2;
			}
		}
		if(target_link!=NULL){
			if(!S_ISLNK(st.st_mode)) return 1;
			char target[1024];
			int length = readlink(pathname, target, 1024);
			if(length<0){
				fprintf(stderr, "Error following symbolic link of file %s: %s\n", pathname, strerror(errno));
				exit(-1);
			}
			target[length] = '\0';
			if(strcmp(realpath(target,NULL), target_link)) return 1;
		}
		if((udisplay>=0) && (udisplay!=st.st_uid))
			return 1;
		if(mdisplay){
			double seconds = difftime(tnow, st.st_mtime);
			return mdisplay>0?(mdisplay>seconds):(mdisplay+seconds>0);
		}
	}
	return 0;
}

void print_info(char *pathname){
		struct stat st;
		struct passwd *pwd;
		struct group *grp;
		struct tm *time;
		char time_buf[17];

		if(lstat(pathname, &st)<0){
			fprintf(stderr, "Error getting information on file %s: %s\n", pathname, strerror(errno));
			exit(-1);
		}

		pwd = getpwuid(st.st_uid);
		grp = getgrgid(st.st_gid);
		time_t t = st.st_mtime; 
		time = localtime(&t);
		strftime(time_buf, 18, "%b %d %Y %H:%M", time);
		
		char mode[11];
		/* The following code was mostly taken from http://stackoverflow.com/questions/10323060/ */
		static char *rwx[] = {"---", "--x", "-w-", "-wx","r--", "r-x", "rw-", "rwx"};
		mode[0] = filetypeletter(st.st_mode);
		strcpy(&mode[1], rwx[(st.st_mode >> 6)& 7]);
		strcpy(&mode[4], rwx[(st.st_mode >> 3)& 7]);
		strcpy(&mode[7], rwx[(st.st_mode & 7)]);
		if (st.st_mode & S_ISUID)
			mode[3] = (st.st_mode & 0100) ? 's' : 'S';
		if (st.st_mode & S_ISGID)
			mode[6] = (st.st_mode & 0010) ? 's' : 'l';
		if (st.st_mode & S_ISVTX)
			mode[9] = (st.st_mode & 0100) ? 't' : 'T';
		mode[10] = '\0';

		if(S_ISLNK(st.st_mode)){
			char target[1024];
			int length = readlink(pathname, target, 1024);
			if(length<0){
				fprintf(stderr, "Error following symbolic link of file %s: %s\n", pathname, strerror(errno));
				exit(-1);
			}
			target[length] = '\0';
			pathname = realloc(pathname,strlen(pathname)+length+5);
			strcat(pathname, " -> ");
			strcat(pathname, target);
		}
		
		printf("%hd/%lu ", st.st_dev,st.st_ino);
		printf("%s %hd ", mode, st.st_nlink); /* this is the number of HARD links */
		printf("%s %s\t", pwd->pw_name, grp->gr_name);
		printf("%ld\t",st.st_size);
		printf("%s ", time_buf);
		printf("%s\n", pathname);

}

void directory(char *direct){
	DIR *dirp;
	struct dirent *de;
	int i;

	if((dirp = opendir(direct))==NULL){
		fprintf(stderr,"Error opening directory %s: %s\n", direct, strerror(errno));
		exit(-1);
	}
	while((de = readdir(dirp))!=NULL){
		char *pathname = malloc(strlen(direct)+strlen(de->d_name)+2);
		if(pathname==NULL){
			fprintf(stderr, "Error Allocating Memory. Please try again. %s\n", strerror(errno));
			exit(-1);
		}
		strcpy(pathname, direct);
		strcat(pathname, "/");
		strcat(pathname, de->d_name);
		
		if(!(i=check_file(pathname, de->d_name))){
			print_info(pathname);
		}			
		/* If device number was different or it is ".","..", stop stepping through the directory */
		if(de->d_type==DT_DIR && i!=2)
			directory(pathname);
	}

	if(closedir(dirp)<0){
		fprintf(stderr, "Error Closing Directory %s: %s\n", direct, strerror(errno));
		exit(-1);
	}
}

int main(int argc, char **argv){
	struct passwd *pwd;
	struct stat st; /* In case of -x, since a declaration is not allowed as part of a label apparently */
	int c = 0;
	while ((c = getopt (argc, argv, "u:m:xl:")) != -1){
		opts = 1;
		switch (c){
			case 'u':
				if(((udisplay = strtol(optarg, NULL, 0))==0) /* if its not a digit */){
					/* String to Int conversion failed because String is not an int */ 
					if((pwd = getpwnam(optarg))==NULL){
						fprintf(stderr, "Error Retrieving User with Username %s\n", optarg);
						exit(-1);
					}
					
					udisplay = pwd->pw_uid;
				}		
				break;
			case 'm':
				 /* tnow is a global variable, in order that the mtime comparison is done with the time at original execution, 
				as opposed to the time at each readdir*/
				time(&tnow);
				mdisplay = atoi(optarg);
				/* If atoi fails, and returns 0, this is okay. There is no difference between mdisplay=0 and no mdisplay declared */
				break;
			case 'x':
				if(stat(argv[argc-1], &st)<0){
					fprintf(stderr, "Error getting information on file %s: %s\n", argv[argc-1], strerror(errno));
					exit(-1);
				}
				vol_start = st.st_dev;
				break;
			case 'l':
				target_link = realpath(optarg, NULL);
				if(target_link==NULL){
					fprintf(stderr, "Error: Target link %s does not exist: %s\n", optarg, strerror(errno));
					exit(-1);
				}
				break;
		}
	}
	directory(argv[argc-1]);
	/* getopt will sort the arguments. Directory will thus be the last entry. If this fails, the error will be found in the directory function */
	return 0;
}

