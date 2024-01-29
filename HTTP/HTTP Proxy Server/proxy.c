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
	char request[MAX],host_name[MAX],response[MAX];
	int i,j,k=0,n,socket_server;
	size_t r;
  	struct addrinfo *hserv,hints,*p;
	memset(request,0,sizeof(request));
	if((r = read(socket_client,request,MAX))>0)
	{
			if(r>0)
			{
				printf("--------Incoming Request-----------\n");
				printf("%s\n",request);
			}
			memset(&host_name,'\0',strlen(host_name));
			for(i=0;i<sizeof(request)-2;i++)
			{
				if(request[i] == '/' && request[i+1] == '/')
				{
					k=0;
					k=i+2;
					for(j=0;request[k] !='/';j++)
					{
						host_name[j]=request[k];
						k++;
					}
					host_name[j]='\0';
					break;
				}
			}
			char* portchr = strstr(host_name,":");
			if(portchr != NULL)
			{
				*portchr = '\0';
				port = portchr + 1;
			}
			printf("Parsed Server hostname: %s\n",host_name);
			printf("Server port: %s\n", port);	
			memset(&hints,'\0',sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;		
			if((n=getaddrinfo(host_name,port,&hints,&hserv))!= 0)
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
					if(connect(socket_server,p->ai_addr,p->ai_addrlen) < 0)
					{
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
			if(send(socket_server,request,MAX,0) < 0)
			{
				perror("Send error...\n");
				exit(-1);
			}
			puts("Request sent to the remote server..\n");
			int byte_count = 0; 
			memset(&response,'\0',sizeof(response));
			while((byte_count = recv(socket_server,response,MAX,0)) > 0)
				send(socket_client,response,byte_count,0);	
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
	printf("\nSocket created...\n");
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
		proxy_to_server(socket_client, "80");
	}
	close(socket_server);
	return 0;
}
