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

int main()
{
	int socket_server, socket_client, port=5001;
	char cmsg[1000],smsg[1000];
	bzero(cmsg,1000);
	bzero(smsg,1000);
	struct sockaddr_in server_address,client_address;
	socklen_t clientlen;
	time_t t;
	if((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\nServer cannot open the socket.");
		exit(1);
	}
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);
	if((bind(socket_server,(struct sockaddr*) &server_address, sizeof(server_address))) < 0)
	{
		printf("\nServer Bind failed.");
		exit(1);
	}
	listen(socket_server,10);
	while(1)
	{
		if((socket_client = accept(socket_server, (struct sockaddr*)&client_address, &clientlen)) < 0)
		{
			printf("\nBad client.");
			exit(1);
		}
		while(strcmp(cmsg,"exit") != 0)
		{
			read(socket_client, cmsg, 1000);
			printf("\nMessage received from the Client is: %s\n", cmsg);
			printf ("\nMessage from Server to Client: ");
			scanf("%s",smsg);
			if(strcmp(smsg,"exit") == 0)
			{
				printf("Disconnected from the Server...\n");
				break;
			}
			write(socket_client, smsg, 1000);
		}
		bzero(cmsg,1000);
		bzero(smsg,1000);
	}
	close(socket_server);
	close(socket_client);
	return 0;
}
