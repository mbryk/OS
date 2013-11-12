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
	int s;
	
	if((s=socket(AF_INET, SOCK_DGRAM, 0))==-1){perror("SOCKET");return -1;}
	struct sockaddr_in sin, server, from;
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr=INADDR_ANY;
	if(bind(s, (struct sockaddr *)&sin, &alen)<0){perror("Bind");close(s);return -1;}
	
	server.sin_family = AF_INET;
	int dest_port;
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
	sendto(s, argv[3], strlen(argv[3]),0, (struct sockaddr *)&server,sizeof server);
	
	int bsize = 1024;
	int alen = sizeof from;
	char *buf = malloc(bsize);
	int buflen;
	do{
		buflen = recvfrom(s, buf, bsize, 0, (struct sockaddr *)&from, &alen);
	} while(from.sin_addr.s_addr != server.sin_addr.s_addr);
	/* Don't want to just take first connection. You want the response from the server. */
	buf[buflen] = '\0';
	printf("Received answer: %s", buf);
	
	return 0;
}
	 
