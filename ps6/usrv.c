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
	if(argc==1){ fprintf(stderr, "Error- Usage: %s <port>\n",argv[0]); return -1; }
	
	int s,port;
	if((s=socket(AF_INET, SOCK_DGRAM, 0))==-1){perror("Socket");return -1;}
	struct sockaddr_in sin, client;
	sin.sin_family = AF_INET;
	if((port=atoi(argv[1]))<1025){
		fprintf(stderr, "Error- Invalid Port #: %s\n", argv[1]);
		return -1;
	}
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;

	if(bind(s, (struct sockaddr *) &sin, sizeof sin)<0){ perror("bind");close(s);return -1;}
	int bsize = 7;
	char *buf = malloc(bsize);
	char *msg;
	int r;
	int alen = sizeof client;
	while(r=recvfrom(s,buf,bsize,0,(struct sockaddr *) &client, &alen)){
		if(!strcmp(buf,"UPTIME")){
			msg = "time";
		}
		else if(!strcmp(buf, "DATE")){
			msg = "date";
		}
		else msg = "Invalid Format";	
		sendto(s, msg, strlen(msg), 0, (struct sockaddr *)&client, alen);
		memset(buf,'\0',bsize);
	}
	if(r==-1){ perror("Receive"); return -1;}
	return 0;
}
