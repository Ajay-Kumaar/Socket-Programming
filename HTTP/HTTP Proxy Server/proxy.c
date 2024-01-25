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

#define MAX 65536
#define PORT 8040

void proxy_to_server(int socket_client, char* port)
{
	char buffer[MAX],host_str1[MAX],serv_response[MAX];
	int i,j,k=0,ti,socket_server;
	size_t r;
  	struct addrinfo *hserv,hints,*p;
	memset(buffer,0,sizeof(buffer));
	if((r = read(socket_client,buffer,MAX))>0)
	{
			if(r>0)
			{
				printf("--------Incoming Request-----------\n");
				printf("%s\n",buffer);
			}
			memset(&host_str1,'\0',strlen(host_str1));
			int tilda =0;
			for(i=0;i<sizeof(buffer)-2;i++)
			{
				if(buffer[i] == '/' && buffer[i+1] == '/')
				{
					k=0;
					k=i+2;
					for(j=0;buffer[k] !='/';j++)
					{
						host_str1[j]=buffer[k];
						k++;
					}
					host_str1[j]='\0';
					break;
				}
			}
			char* portchr = strstr(host_str1,":");
			if(portchr != NULL)
			{
				*portchr = '\0';
				port = portchr + 1;
			}
			printf("Parsed Hostname from the Client HTTP request: %s\n",host_str1);
			printf("Server port: %s\n", port);	
			memset(&hints,'\0',sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;		
			if((ti=getaddrinfo(host_str1,port,&hints,&hserv))!= 0)
			{
				herror("Getaddrinfo error...\n");
				exit(-1);
			}
			for(p = hserv;p!=NULL;p = p->ai_next)
			{
					socket_server = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
					if(socket_server<0)
					{
						perror("Socket creation error...\n");
						continue;
					}				
					
					if(connect(socket_server,p->ai_addr,p->ai_addrlen) < 0){
						close(socket_server);
						perror("Connection error...\n");
						continue;				
					}
					break;	
			}
			if(p == NULL)
			{
				printf("Not connected\n");
				exit(-1);			
			}
			puts("Connected to the remote server...\n");
			if(send(socket_server,buffer,1024,0) < 0)
			{
				perror("Send error...\n");
				exit(-1);
			}
			puts("Request sent to the remote server..\n");
			int byte_count = 0; 
			memset(&serv_response,'\0',sizeof(serv_response));
			while((byte_count = recv(socket_server,serv_response,MAX,0)) > 0){
					send(socket_client,serv_response,byte_count,0);	
					//printf("%s\n",serv_response);
			}
			printf("Data completely sent to the web browser through HTTP Proxy...\n");
			printf("-----------------------------------------------------------");
			close(socket_server);	
		}
		close(socket_client);
}
int main()
{
	int socket_server,socket_client;
	socklen_t cli_len;
	size_t l;
	struct sockaddr_in server_address,client_address;
	socket_server = socket(AF_INET,SOCK_STREAM,0);
	if(socket_server == -1)
	{
		perror("Socket creation error...\n");
		exit(-1);
	}
	memset(&server_address,0,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
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
	printf("Proxy server listening at port %d...\n",PORT);
	cli_len = sizeof(client_address);
	while(1)
	{
		socket_client = accept(socket_server,(struct sockaddr*) &client_address,&cli_len);
		if(socket_client == -1)
		{
			perror("Client connection error...\n");
			exit(-1);		
		}
		printf("\nClient connected...\n");
		char arr[100] = "80";
		proxy_to_server(socket_client, arr);
	}
	close(socket_server);
	return 0;
}
