/* Mark Bryk, OS PS6, TCP_SEND.C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

int main(int argc, char **argv){
	if(argc<3){ fprintf(stderr, "Error- Usage: %s <hostname> <port>\n",argv[0]); return -1;}

	int s;
	if((s=socket(AF_INET, SOCK_STREAM,0))==-1){perror("Socket"); return -1;}
	struct linger sol;
	sol.l_onoff = 1; sol.l_linger = 30; /* Max Wait Time = 30 seconds */
	if(setsockopt(s,SOL_SOCKET,SO_LINGER,&sol,sizeof sol)){
		perror("Error Setting SO_LINGER"); return -1;
	}
	
	struct sockaddr_in sin, dest;
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr=INADDR_ANY;
	if(bind(s, (struct sockaddr *)&sin, sizeof sin)<0){
		perror("bind");
		close(s);
		return -1;
	}
	
	dest.sin_family = AF_INET;

	int dest_port;
	if((dest_port=atoi(argv[2]))==0){
		fprintf(stderr,"Error- Invalid Port #: %s\n",argv[2]);
		return -1;
	}
	dest.sin_port = htons(dest_port);

	if((dest.sin_addr.s_addr=inet_addr(argv[1]))==-1){
		struct hostent *he;
		if(!(he=gethostbyname(argv[1]))){
			fprintf(stderr, "Unknown host: %s\n",argv[1]);
			herror(" ");
			return -1;			
		}
		memcpy(&dest.sin_addr.s_addr,he->h_addr_list[0],sizeof dest.sin_addr.s_addr);
	}

	if(connect(s, (struct sockaddr *)&dest, sizeof dest)<0){
		perror("connect"); close(s); return -1;
	}

	int r,r_full,n;
	int bsize = 4096;
	int bytes = 0;
	char *buf = malloc(bsize);
	char *buf_tmp;
	struct timeval begin,end;
	if(gettimeofday(&begin,NULL)==-1){ perror("Error recording begin time of write to socket"); return -1; }
	while((r = read(0,buf,bsize))!=0){
		if(r==-1){ perror("Error reading from input"); return -1; }
		buf_tmp = buf; r_full = r;
		while((n=write(s,buf_tmp,r))!=r && n!=-1){
			buf_tmp = buf_tmp+n;
			r-=n;
		}
		if(n==-1){ perror("Error writing to socket"); return -1; }
		bytes+=r_full;
	}
	if(close(s)<0){
		if(errno==EWOULDBLOCK) perror("Not All Data Sent");
		else { perror("Error Closing Socket"); return -1;}
	}
	if(gettimeofday(&end,NULL)==-1){ perror("Error recording end time of write to socket"); return -1; }
	
	double secs = difftime(end.tv_sec,begin.tv_sec);
	double usecs = difftime(end.tv_usec,begin.tv_usec)/1000000;
	secs += usecs; /* This is fine, regarding the carry. If begin.usec>end.usec, then usecs is negative and secs is decremented properly. */
	double rate = bytes/secs/1048576;
	fprintf(stderr, "Seconds:\t%f\n",secs);
	fprintf(stderr, "Bytes Sent:\t%d\nTransfer Rate:\t%f MB/s\n",bytes,rate);
	return 0;
}
