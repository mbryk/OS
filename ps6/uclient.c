/* Mark Bryk, OS PS6, UDP_CLIENT.C */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv){
	if(argc<4){ fprintf(stderr, "Error- Usage: %s <hostname> <port> <request_string>\n", argv[0]); return -1;}
	int s,dest_port;
	struct sockaddr_in sin, server, from;
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr=INADDR_ANY;

	server.sin_family = AF_INET;
	if((dest_port=atoi(argv[2]))==0){
		fprintf(stderr,"Error- Invalid Port #: %s\n",argv[2]);
		return -1;
	}
	server.sin_port = htons(dest_port);
	if((server.sin_addr.s_addr=inet_addr(argv[1]))==-1){
		struct hostent *he;
		if(!(he=gethostbyname(argv[1]))){
			fprintf(stderr, "Unknown host: %s\n",argv[1]);
			herror(" ");
			return -1;
		}
		memcpy(&server.sin_addr.s_addr,he->h_addr_list[0],sizeof server.sin_addr.s_addr);
	}	

	if((s=socket(AF_INET, SOCK_DGRAM, 0))==-1){perror("Error Creating Socket");return -1;}
	if(bind(s, (struct sockaddr *)&sin, sizeof sin)<0){perror("Error Binding socket to sockaddr");close(s);return -1;}
	if(sendto(s, argv[3], strlen(argv[3]),0, (struct sockaddr *)&server,sizeof server)<0){perror("Error Sending UDP Packet to server"); close(s); return -1;}
	
	int bsize = 256; //Not expecting a big message back.
	char *buf = malloc(bsize);
	if(buf==NULL){perror("Error Allocating Memory for buffer"); close(s); return -1;}
	int buflen;
	int alen = sizeof from;
	do{
		buflen = recvfrom(s, buf, bsize, 0, (struct sockaddr *)&from, &alen);
	} while(from.sin_addr.s_addr != server.sin_addr.s_addr);
	/* Don't want to just take first connection. You want the response from specifically the server. */
	buf[buflen] = '\0';
	printf("Received answer: %s", buf);

	close(s); free(buf);
	return 0;
}