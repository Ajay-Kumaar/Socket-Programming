#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define LOCALHOST "10.39.17.249"
#define PORT 8080
#define FILE_DIR "/var/www/html"
#define MAX_BUFFER_SIZE 1024

void send_http_response(int socket_client, const char* response)
{
    send(socket_client, response, strlen(response), 0);
}
void handle_http_request(int socket_client, const char* request)
{
	FILE* file = NULL;
    char method[10], resource[MAX_BUFFER_SIZE];
    sscanf(request, "%s %s", method, resource);
	char* temp = strstr(resource,"//");
	if(temp != NULL)
		temp = strstr(temp+2,"/");
	else
		temp = resource;
	if(strcmp(temp,"/") == 0)
		file = fopen("/var/www/html/index.nginx-debian.html", "r");
	else
	{
    	char file_path[sizeof(FILE_DIR) + MAX_BUFFER_SIZE];
    	snprintf(file_path, sizeof(file_path), "%s%s", FILE_DIR, temp);
    	file = fopen(file_path, "r");
	}
    if (file != NULL)
	{
        const char* response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:1024\r\n\r\n";
        send_http_response(socket_client, response_header);
        char file_buffer[2*MAX_BUFFER_SIZE];
        size_t bytesRead;
        while ((bytesRead = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0)
            send(socket_client, file_buffer, bytesRead, 0);
        fclose(file);
    }
	else
	{
        const char* response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n404 - Not Found\n";
        send_http_response(socket_client, response);
    }
}
int main (int argc, char* argv[])
{
	int socket_server,socket_client;
	char request[MAX_BUFFER_SIZE];
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
	server_address.sin_addr.s_addr = INADDR_ANY;
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
		printf("\nConnection to Client successful...\n");
		if(read(socket_client,request,sizeof(request)) < 0)
		{
			printf("HTTP request read operation failed...\n");
			exit(1);
		}
		printf("\nRequest from the HTTP Client: %s\n",request);
		handle_http_request(socket_client, request);
		close(socket_client);
	}
	return 0;
}
