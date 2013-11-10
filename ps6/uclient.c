/* Mark Bryk, OS PS6, UDP_CLIENT.C */
#include <sys/type.h>
#include <sys/socket.h>

int main(int argc, char **argv){
	dest_addr = from argv[1] (hostname gethostbyname, or numbers aton)
	dest_port = from argv[2]
	msg = argv[3];
	
	if((s=socket(AF_INET, SOCK_DGRAM, 0))==-1){perror("SOCKET");return -1;}
	struct sockaddr sin, to;
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr=INADDR_ANY;
	if(bind(s, (struct sockaddr *)&sin, &alen)<0){perror("Bind");close(s);return -1;}
	
	to.sin_family = AF_INET;
	to.sin_port = dest_port;
	to.sin_addr.s_addr = dest_addr;
	sendto(s, msg, 0, (struct sockaddr *)&to,sizeof to);
	int bsize = 1024;
	int alen;
	char buf[bsize];
	struct sockaddr doesntmatter;

	recvfrom(s, buf, bsize, 0, (struct sockaddr *)&doesntmatter, &alen);

	 
