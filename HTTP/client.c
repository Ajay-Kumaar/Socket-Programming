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
	if(argc != 2)
	{
		printf ("\nInvalid format...Expected format: program_name IP_Address");
		exit(1);
	}
	if((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("\nSocket cannot be opened.");
		exit(1);
	}
	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9000);
	inet_aton(argv[1], server_address.sin_addr.s_addr);
	if(connect(socket_client, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		printf ("\nConnection Failed.");
		exit(1);
	}
	printf ("\nClient-Server connected...\n");
	char request[100] = "GET / HTTP/1.1\n\n";
	char response[2000];
	write(socket_client,request,100);
	read(socket_client,response,2000);
	printf("\nResponse from the Server: %s\n",response);
	close(socket_client);
	return 0;
}
