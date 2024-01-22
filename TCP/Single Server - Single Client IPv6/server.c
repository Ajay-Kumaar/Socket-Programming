#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{
	int socket_server, socket_client;
	char cmsg[100], smsg[100] = "This is the response from the Server...";
	struct sockaddr_in6 server_address,client_address;
	socklen_t clientlen;
	if((socket_server = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
	{
		printf("\nServer cannot open the socket.");
		exit(1);
	}
	bzero(&server_address, sizeof(server_address));
	server_address.sin6_family = AF_INET6;
	server_address.sin6_port = htons(8000);
	//server_address.sin6_addr.s6_addr = inet_addr("::1");
	//inet_pton(AF_INET6,“fe80::f665:2273:bca8:34a6”,(void *)&server_address.sin6_addr.s6_addr);
	//inet_pton(AF_INET6, “::1”, (void *)(&server_address.sin6_addr));
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
	read(socket_client, cmsg, 100);
	printf("\nMessage received from the Client is: %s\n", cmsg);
	write(socket_client, smsg, 100);
	close(socket_server);
	close(socket_client);
	return 0;
}
