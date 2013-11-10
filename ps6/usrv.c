/* Mark Bryk, OS PS6, UDP_SERVER.C */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv){
	if(argc==1){ fprintf(stderr, "Error- Incorrect Input. Must include listening port\n"); return -1; }
	
	int s,port;
	if((s=socket(AF_INET, SOCK_DGRAM, 0))==-1){perror("Socket");return -1;}
	struct sockaddr_in sin, from;
	sin.sin_family = AF_INET;
	if((port=atoi(argv[1]))<1025){
		fprintf(stderr, "Error- Invalid Port #: %s\n", argv[1]);
		return -1;
	}
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;

	if(bind(s, (struct sockaddr *) &sin, sizeof sin)<0){ perror("bind");close(s);return -1;}
	int bsize = 6;
	char *buf = malloc(bsize);
	char *msg;
	int r, alen;
	while(r=recvfrom(s,buf,bsize,0,(struct sockaddr *) &from, &alen)){
		msg = strcmp(buf,"UPTIME")?"date":"time";	
		sendto(s, msg, strlen(msg), 0, (struct sockaddr *)&from, alen);
	}
	return 0;
}
