/* Mark Bryk, OS PS6, UDP_SERVER.C */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

int main(int argc, char **argv){
	if(argc==1){ fprintf(stderr, "Error- Usage: %s <port>\n",argv[0]); return -1; }
	
	int s,port,r;
	struct sockaddr_in sin, client;
	sin.sin_family = AF_INET;
	if((port=atoi(argv[1]))<1025){
		fprintf(stderr, "Error- Invalid Port #: %s\n", argv[1]);
		return -1;
	}
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;

	if((s=socket(AF_INET, SOCK_DGRAM, 0))==-1){perror("Error Creating Socket");return -1;}
	if(bind(s, (struct sockaddr *) &sin, sizeof sin)<0){ perror("Error Binding socket to sockaddr");close(s);return -1;}
	int bsize = 7;
	char *buf = malloc(bsize);
	if(buf==NULL){perror("Error Allocating Memory for buffer"); close(s); return -1;}
	char *command = NULL;
	char *msg;
	int alen = sizeof client;
	while(r=recvfrom(s,buf,bsize,0,(struct sockaddr *) &client, &alen)){
		if(!strcmp(buf,"UPTIME")) command = "uptime";
		else if(!strcmp(buf,"DATE")) command = "date";
		else msg = "Invalid Command Format\n";	
		if(command != NULL){
			FILE *fp;
			int length; size_t size = 0;
			if((fp=popen(command,"r"))!=NULL){
				if((length=getline(&msg, &size, fp))!=-1)
					pclose(fp);
				else msg = "Internal Server Error\n";
			} else msg = "Internal Server Error\n";
		}
		sendto(s, msg, strlen(msg), 0, (struct sockaddr *)&client, alen);
		memset(buf,'\0',bsize); command = NULL; /* Reset vars for recvfrom and strcmp */
	}
	if(r==-1){ perror("Error Receiving UDP Packet"); close(s); return -1;}
	close(s); free(buf);
	return 0;
}
