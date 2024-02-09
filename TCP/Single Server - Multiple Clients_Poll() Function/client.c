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

#define SERVER_PORT 8000
#define MAX_BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
	int client_socket;
	struct sockaddr_in server_address;
	char cmsg[MAX_BUFFER_SIZE],smsg[MAX_BUFFER_SIZE];
	bzero(cmsg,sizeof(cmsg));
	bzero(smsg,sizeof(smsg));
	time_t t;
	if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("\nSocket cannot be opened.");
		exit(1);
	}
	printf("Client socket created.\n");
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SERVER_PORT);
	if(connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		printf ("\nConnection Failed.");
		exit(1);
	}
	printf("Client connected to the Echo Server.\n");
	while(strcmp(cmsg,"exit") != 0)
	{
		printf("\nMessage from Client to Server: ");
		scanf("%s",cmsg);
		send(client_socket,cmsg,strlen(cmsg),0);
		recv(client_socket,smsg,sizeof(smsg),0);
		printf("Message received from the Server is: %s\n", smsg);
	}
	close(client_socket);
	return 0;
}
