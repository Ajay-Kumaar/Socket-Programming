#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<strings.h>
#include<time.h>

int main (int argc, char* argv[])
{
	int sockfd, port=5001;
	char cmsg[100], smsg[100];
	struct sockaddr_in server_address;
	if (argc != 3)
	{
		printf ("\nInvalid Format.");
		exit(1);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("\nSocket cannot be opened.");
		exit(1);
	}
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[2]));
	if (connect(sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		printf ("\nConnection Failed.");
		exit(1);
	}
	printf ("\nClient-Server connected...\n");
	printf ("\nMessage from Client to Server: ");
	scanf("%s",cmsg);
	write(sockfd,cmsg,100);
	time_t t;
	time(&t);
	printf("\nTime: %s\n", ctime(&t));
	int n = read(sockfd,smsg,100);
	printf("\nMessage received from the Server is: %s\n", smsg);
	time(&t);
	printf("\nTime: %s\n", ctime(&t));
	close(sockfd);
	return 0;
}
