#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<error.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>

#define MAX 653320

void proxy_to_server(int socket_client)
{
	char buffer[MAX],host_str1[MAX],serv_response[MAX];
	int i,j,k=0,ti,socket_server;
	size_t r;
	socklen_t cli_len,serv_len;
  	struct addrinfo *hserv,hints;
	struct sockaddr_in remote_server_address;
	memset(buffer,0,sizeof(buffer));
	if((r = read(socket_client,buffer,MAX))>0)
	{
		if(r>0)
		{
			printf("--------Incoming Request-----------\n");
			printf("%s\n",buffer);
		}
		memset(host_str1,0,strlen(host_str1));
		int tilda =0;
		for(i=0;i<sizeof(buffer)-2;i++)
		{
			if(buffer[i] == '/'&& buffer[i+1] == '/')
			{
				k=0;
				k=i+2;
				for(j=0;buffer[k] !='/';j++)
				{
					host_str1[j]=buffer[k];
					printf("%c\n",host_str1[j]);
					k++;
				}
				tilda++;
			}
			if(tilda == 1)
				goto label1;
		}
		label1:
		printf("Parsed URL from the Client HTTP request: %s\n",host_str1);		
		hints.ai_family = INADDR_ANY;
		hints.ai_socktype = SOCK_STREAM;		
		if((ti=getaddrinfo(host_str1,"80",&hints,&hserv))!= 0)
		{
			herror("Getaddrinfo error...\n");
			exit(-1);
		}
		socket_server = socket(AF_INET,SOCK_STREAM,0);
		if(socket_server<0)
		{
			perror("Socket creation error...\n");
			exit(-1);
		}
		memset(&remote_server_address,0,sizeof(remote_server_address));
		remote_server_address.sin_family = AF_INET;
		remote_server_address.sin_port = htons(80);
		if(connect(socket_server,(struct sockaddr *) hserv->ai_addr,hserv->ai_addrlen) < 0)
		{
			perror("Connection error...\n");
			exit(-1);
		}
		puts("Connected to the remote server...\n");
		if(send(socket_server,buffer,strlen(buffer),0) < 0)
		{
			perror("Send error...\n");
			exit(-1);
		}
		memset(buffer,0,sizeof(buffer));
		puts("Request sent to the remote server..\n");
		int byte_count,byte_sent; 
		memset(serv_response,0,sizeof(serv_response));
		do
		{
			byte_count = recv(socket_server,serv_response,sizeof(serv_response),0);
			printf("Byte Count:%d\n",byte_count);	
			if(!(byte_count <= 0))
			{	
				byte_sent = send(socket_client,serv_response,byte_count,0);
				printf("%d bytes of data sent to web browser...\n",byte_sent);
			}
		}
		while(byte_count > 0); 
		puts("Data completely sent to the web browser through HTTP Proxy...\n");
		close(socket_server);	
	}		
}
int main(int argc,char* argv[])
{
	int socket_server,socket_client;
	socklen_t cli_len;
	size_t l;
	struct sockaddr_in server_address,client_address;
	if(argc < 2)
	{
		fprintf(stderr,"Error! Please enter the port");
		exit(1);
	}
	socket_server = socket(AF_INET,SOCK_STREAM,0);
	if(socket_server == -1)
	{
		perror("Socket creation error...\n");
		exit(-1);
	}
	memset(&server_address,0,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[1]));
	server_address.sin_addr.s_addr = INADDR_ANY;
	printf("Socket created...\n");
	if(bind(socket_server,(struct sockaddr*)&server_address,sizeof(server_address)) < 0)
	{	
		perror("Bind error...\n");
		exit(-1);
	}
	printf("Bind successful...\n");
	if(listen(socket_server,5) == -1)
	{
		perror("listen error...\n");
		exit(-1);
	}
	cli_len = sizeof(client_address);
	//again:
	socket_client = accept(socket_server,(struct sockaddr*) &client_address,&cli_len);
	if(socket_client == -1)
	{
		perror("Client connection error...\n");
		exit(-1);		
	}
	printf("Client connected...\n");
	proxy_to_server(socket_client);
	close(socket_server);
	//goto again;
	return 0;
}
