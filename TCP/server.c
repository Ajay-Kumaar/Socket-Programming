#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<strings.h>
#include<time.h>

int main()
{
	int socket_server, socket_client, port=5000;
	char cmsg[100], smsg[100];
	struct sockaddr_in server_address,client_address;
	socklen_t clientlen;
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
	listen(socket_server,5);
	if((socket_client = accept(socket_server, (struct sockaddr*) &client_address, &clientlen)) < 0)
	{
		printf("\nClient is Bad.");
		exit(1);
	}
	int n = read(socket_client, cmsg, 100);
	printf("\nMessage received from the Client is: %s\n", cmsg);
	time_t t;
	time(&t);
	printf("\nTime: %s\n", ctime(&t));
	printf ("\nMessage from Server to Client: ");
	scanf("%s",smsg);
	write(socket_client, smsg, 100);
	time(&t);
	printf("\nTime: %s\n", ctime(&t));
	close(socket_server);
	close(socket_client);
	return 0;
}
