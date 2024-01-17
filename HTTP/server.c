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
	FILE* fp = fopen("index.html","r");
	char response[1000];
	fgets(response,1000,fp);
	char header[2000] = "HTTP/1.1 200 OK\n";
	strcat(header,response);
	int socket_server,socket_client;
	if((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("\nSocket cannot be opened.");
		exit(1);
	}
	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9000);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	if((bind(socket_server,(struct sockaddr*) &server_address, sizeof(server_address))) < 0)
	{
		printf("\nServer Bind failed.");
		exit(1);
	}
	listen(socket_server,5);
	while(1)
	{
		if((socket_client = accept(socket_server, NULL, NULL)) < 0)
		{
			printf("\nClient is Bad.");
			exit(1);
		}
		write(socket_client, header, 2000);
		close(socket_client);
	}
	close(socket_server);
	return 0;
}
