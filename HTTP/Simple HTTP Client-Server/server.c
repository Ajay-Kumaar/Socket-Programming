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

int main (int argc, char* argv[])
{
	FILE* fp = fopen("index.html","r");
	char response[1000];
	fgets(response,1000,fp);
	fclose(fp);
	//char header[2000] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	char header[1024] = "HTTP/1.1 200 OK\r\n\n";	
	strcat(header,response);
	int socket_server,socket_client;
	if((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("Socket cannot be opened...\n");
		exit(1);
	}
	printf ("\nSocket opened successfully...\n");
	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = inet_addr(LOCALHOST);
	if((bind(socket_server,(struct sockaddr*) &server_address, sizeof(server_address))) < 0)
	{
		printf("Server Bind failed...\n");
		exit(1);
	}
	printf("Server bind success...\n");
	if(listen(socket_server,5) < 0)
	{
		printf("Server listen function failed...\n");
		exit(1);
	}
	printf("Server listening at port: %d\n", PORT);
	while(1)
	{
		if((socket_client = accept(socket_server, NULL, NULL)) < 0)
		{
			printf("Connection to Client failed...\n");
			exit(1);
		}
		printf("Connection to Client successful...\n");
		write(socket_client, header, sizeof(header));
		close(socket_client);
	}
	return 0;
}
