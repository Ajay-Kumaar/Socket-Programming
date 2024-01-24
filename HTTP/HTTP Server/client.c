#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define LOCALHOST "127.0.0.1"
#define PORT 8080
#define MAXSIZE 1024

int main (int argc, char* argv[])
{
	int socket_client;
	/*if(argc != 2)
	{
		printf ("\nInvalid format...Expected format: program_name IP_Address_of_the_Server"); This is for explicit Server IP address provided in the command line
		exit(1);
	}*/
	if((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("Socket cannot be opened...\n");
		exit(1);
	}
	printf ("\nSocket opened successfully...\n");
	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	if(argc == 1)
	{
		server_address.sin_port = htons(8080);
		server_address.sin_addr.s_addr = inet_addr(LOCALHOST);
	}
	else
	{
		server_address.sin_port = htons(80);
		inet_pton(AF_INET, argv[1], &server_address.sin_addr);
	}
	if(connect(socket_client, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		printf ("Connection failed...\n");
		exit(1);
	}
	printf("Client connected successfully to the Server %s:%d\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port));
	char request[] = "GET / HTTP/1.1\r\n\r\n";
	char response[MAXSIZE];
	bzero(response,MAXSIZE);
	write(socket_client,request,sizeof(request));
	printf("\nResponse from the Server:\n\n");
	while(read(socket_client,response,MAXSIZE) > 0)
	{
		printf("%s",response);
		bzero(response,MAXSIZE);
	}
	close(socket_client);
	return 0;
}
