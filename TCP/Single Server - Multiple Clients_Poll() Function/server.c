#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/poll.h>

#define SERVER_PORT 8000
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main(int argc, char* argv[])
{
    int server_socket, client_socket;
    struct sockaddr_in server_address;
	struct sockaddr_in client_address[MAX_CLIENTS + 1];
    socklen_t client_address_len = sizeof(client_address);
    char buffer[MAX_BUFFER_SIZE];
	int yes=1,j=1;
	time_t t;
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
	printf("Server socket created.\n");
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(0);	
	}
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
	{
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
	printf("Server bind successful.\n");
    if(listen(server_socket, MAX_CLIENTS) == -1)
	{
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening at port %d.\n", SERVER_PORT);
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;
    fds[0].fd = server_socket;
    fds[0].events = POLLIN;
    while(1)
	{
        if(poll(fds, nfds, -1) == -1)
		{
            perror("Poll failed");
            exit(EXIT_FAILURE);
        }
		printf("\npoll() successful.\n");
        if(fds[0].revents & POLLIN)
		{
            if((client_socket = accept(server_socket, (struct sockaddr*)&client_address[nfds], &client_address_len)) == -1)
			{
                perror("Accept failed");
                continue;
            }
            printf("\nConnection from Client %d, [%s:%d]\n\n", j++, inet_ntoa(client_address[nfds].sin_addr), ntohs(client_address[nfds].sin_port));
            fds[nfds].fd = client_socket;
            fds[nfds].events = POLLIN;
            nfds++;
        }
        for(int i = 1; i < nfds; ++i)
		{
            if(fds[i].revents & POLLIN)
			{
				bzero(buffer,sizeof(buffer));
                int bytes_received = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                if(bytes_received <= 0)
				{
                    printf("\nConnection closed by the Client %s:%d\n\n", inet_ntoa(client_address[i].sin_addr), ntohs(client_address[i].sin_port));
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];
					client_address[i] = client_address[nfds - 1];
                    nfds--;
                }
				else
				{
					printf("Message received from the Client [%s:%d] is %s.\n",inet_ntoa(client_address[i].sin_addr), ntohs(client_address[i].sin_port),buffer);
					printf("Echoing the message back to the Client along with the timestamp...\n\n");
					time(&t);
					strcat(buffer,ctime(&t));
                    send(fds[i].fd, buffer, strlen(buffer), 0);
				}
            }
        }
    }
    close(server_socket);
    return 0;
}
