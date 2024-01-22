#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main (int argc, char* argv[])
{
	int sockfd;
	char cmsg[100],smsg[100];
	struct sockaddr_in6 server_address;
	if((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
	{
		printf ("\nSocket cannot be opened.");
		exit(1);
	}
	bzero(&server_address, sizeof(server_address));
	server_address.sin6_family = AF_INET6;
	server_address.sin6_port = htons(8000);
	if(connect(sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		printf ("\nConnection Failed.");
		exit(1);
	}
	printf ("\nClient-Server connected...\n");
	printf ("\nMessage from Client to Server: ");
	scanf("%[^\n]s",cmsg);
	write(sockfd,cmsg,100);
	read(sockfd,smsg,100);
	printf("\nMessage received from the Server is: %s\n", smsg);
	close(sockfd);
	return 0;
}
