/* Mark Bryk, OS PS6, TCP_RECV.C */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv){
	if(argc==1){ fprintf(stderr, "Error- Usage: %s <Port #>\n",argv[0]); return -1;}

	int s,s2,port;
	if((s=socket(AF_INET, SOCK_STREAM,0))==-1){perror("Socket"); return -1;}

        struct sockaddr_in sin, from;
        sin.sin_family = AF_INET;
	if((port=atoi(argv[1]))<1025){
		fprintf(stderr, "Error- Invalid Port #: %s\n", argv[1]);
		return -1;
	}
        sin.sin_port = htons(port);
        sin.sin_addr.s_addr=INADDR_ANY;

        if(bind(s, (struct sockaddr *)&sin, sizeof sin)<0){
                perror("bind");
                close(s);
                return -1;
        }
	if(listen(s,128)<0){ perror("Listen"); return -1; }
	int len = sizeof from;
        if((s2=accept(s, (struct sockaddr *)&from,&len))<0){
                perror("accept"); return -1;
        }

	int r,r_full,n;
        int bsize = 4096;
        int bytes = 0;
        char *buf = malloc(bsize);
        char *buf_tmp;
        struct timeval begin,end;
        if(gettimeofday(&begin,NULL)==-1){ perror("Error recording begin time of read from socket"); return -1; }
        while((r = read(s2,buf,bsize))!=0){
                if(r==-1){ perror("Error reading from socket"); return -1; }
                buf_tmp = buf; r_full = r;
                while((n=write(1,buf_tmp,r))!=r && n!=-1){
                        buf_tmp = buf_tmp+n;
                        r-=n;
                }
                if(n==-1){ perror("Error writing to output"); return -1; }
                bytes+=r_full;
        }
	close(s);close(s2);
        if(gettimeofday(&end,NULL)==-1){ perror("Error recording end time of read from socket"); return -1; }

        double secs = difftime(end.tv_sec,begin.tv_sec);
        double usecs = difftime(end.tv_usec,begin.tv_usec)/1000000;
        secs += usecs; /* This is fine, regarding the carry. If begin.usec>end.usec, then usecs is negative and secs is decremented properly. */
        double rate = bytes/secs/1000000;

	struct hostent *he;
	if(!(he=gethostbyaddr((char *)&from.sin_addr, sizeof from.sin_addr, AF_INET))){
		fprintf(stderr, "Error retrieving information on TCP sender\n");
		herror(" ");
		return -1;
	}
	char *ip = inet_ntoa(from.sin_addr);
	fprintf(stderr, "Remote Endpoint:\n\tIP address: %s\n",ip);
	if(he->h_name) fprintf(stderr, "\tName: %s\n", he->h_name);
	fprintf(stderr, "\tPort: %d\n", ntohs(from.sin_port));
        fprintf(stderr, "Bytes Sent:\t%d\nTransfer Rate:\t%f MB/s\n",bytes,rate);
        return 0;
}
