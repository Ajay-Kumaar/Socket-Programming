#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main (int argc, char* argv[])
{
	int socket_client;
	char cmsg[100],smsg[100];
	struct sockaddr_in server_address;
	time_t t;
	if(argc != 3)
	{
		printf ("\nInvalid Format.");
		exit(1);
	}
	if((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("\nSocket cannot be opened.");
		exit(1);
	}
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[2]));
	if(connect(socket_client, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		printf ("\nConnection Failed.");
		exit(1);
	}
	printf ("\nClient-Server connected...\n");
	while(1)
	{
		printf ("\nMessage from Client to Server: ");
		scanf("%[^\n]s",cmsg);
		write(socket_client, cmsg, 100);
		read(socket_client,smsg,100);
		if(strcmp(smsg,"exit") == 0)
		{
			close(socket_client);
			printf("\nDisconnected from the Server...\n");
			exit(1);
		}
		printf("\nMessage received from the Server is: %s\n", smsg);
	}
	close(socket_client);
	return 0;
}
