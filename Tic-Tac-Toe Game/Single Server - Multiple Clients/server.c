#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8000
#define MAX_CLIENTS 10

typedef struct
{
	int id;
	int opp_id;
    int socket;
    char symbol;
	char board[3][3];
}Player;

int client_sockets[MAX_CLIENTS];
int active_users[MAX_CLIENTS] = {0};
int n = 0;

void displayBoard(char board[3][3])
{
    printf("Current Tic-Tac-Toe Board:\n");
    printf("-------------\n");
    for (int i = 0; i < 3; i++)
	{
        for (int j = 0; j < 3; j++)
            printf("| %c ", board[i][j]);
        printf("|\n-------------\n");
    }
}

int isBoardFull(char board[3][3])
{
    for (int i = 0; i < 3; i++)
	{
        for (int j = 0; j < 3; j++)
		{
            if (board[i][j] == ' ')
                return 0;
        }
    }
    return 1;
}

int checkWinner(char board[3][3], char symbol)
{
    for (int i = 0; i < 3; i++)
	{
        if ((board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol) ||
            (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol))
            return 1;
    }
    if ((board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol) ||
        (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol))
        return 1;
    return 0;
}

int main()
{
    int server_socket, max_sockfd, activity, sd, yes = 1;
    struct sockaddr_in server_addr;
    fd_set readfds;
    Player player[MAX_CLIENTS];
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
	{
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	printf("Server socket created.\n");
    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
	printf("Server bind success.\n");
    if (listen(server_socket, MAX_CLIENTS) < 0)
	{
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening at port %d.\n", PORT);
    while (1)
	{
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sockfd = server_socket;
        for (int i = 0; i < MAX_CLIENTS; i++)
		{
            sd = client_sockets[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sockfd)
                max_sockfd = sd;
        }
        activity = select(max_sockfd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
            perror("Select error");
        if (FD_ISSET(server_socket, &readfds))
		{
            int client_socket;
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
            if (client_socket < 0)
			{
                perror("Accept error");
                exit(EXIT_FAILURE);
            }
            printf("\n\nNew player connected. Details:\nSocket File Descriptor: %d\nIP Address: %s\nPort: %d\n\n",
                   client_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			player[n].id = n;
			player[n].socket = client_socket;
			active_users[n] = 1;
			n++;
			//printf("\n%d\n", n);
            for (int i = 0; i < n; i++)
			{
                if (client_sockets[i] == 0)
				{
                    client_sockets[i] = client_socket;
                    break;
                }
            }
        }
        for (int i = 0; i < n; i++)
		{
            sd = client_sockets[i];
			char buffer[1024];
			char msg[1024];
            if (FD_ISSET(sd, &readfds))
			{
				bzero(buffer, sizeof(buffer));
                ssize_t bytes_received = recv(sd, buffer, sizeof(buffer), 0);
				//printf("\n%ld   %s\n", bytes_received, buffer);
				if (bytes_received == 0)
				{
                   	printf("Player %d disconnected from the game.\n", i);
                    close(sd);
                    client_sockets[i] = 0;
					//n--;
                }
				else
				{
                    buffer[strlen(buffer)] = '\0';
                    int move, row, col;
					bzero(msg, sizeof(msg));
                    if(strstr(buffer, "active_players") != NULL)
					{
						//sprintf(msg, "List of active players:\n");
						//printf("\n%d\n", n);
						char active[n];
						bzero(active, sizeof(active));
						int k = 0;
						for(int j=0; j<n; j++)
						{
							//printf("\n%d\n", j);
							if(j  == i)
								continue;
							if(active_users[j] == 1){
								//sprintf(msg, "%d\n", j);
								//printf("\n%d\n", j);
								active[k++] = j + '0';
							}
						}
						printf("Hi");
						printf("%s", active);
						send(sd, "List of active players:\n", sizeof("List of active players:\n"), 0);
						printf("%s", active);
						//send(sd, active, sizeof(active), 0);
					}
					else if(strstr(buffer, "game_request") != NULL)
					{
						char* ptr = strstr(buffer, " ");
						ptr+=1;
						int id = atoi(ptr);
						send(client_sockets[i], "Request sent.\n", strlen("Request sent.\n"), 0);
						sprintf(msg, "Game request from player %d.\n",i);
						send(client_sockets[id], msg, strlen(msg), 0);
					}
					else if(strcmp(buffer, "accept_request") == 0)
					{
						
					}
                }
            }
        }
    }
    return 0;
}
