#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8000
#define MAX_BUFFER_SIZE 1024

char board[3][3];

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

int main()
{
    int client_socket;
	char move;
	int ingame = 0, activity;
	char buffer[MAX_BUFFER_SIZE];
	char message[MAX_BUFFER_SIZE];
	ssize_t bytes_received = 0;
    struct sockaddr_in server_addr;
	fd_set readfds;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
	printf("Client socket created.\n");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    printf("You are connected to the server.\n\n");
	printf("\n\nType -- active_players -- to get the list of active players available to play a new game\nType -- game_request \'opponent_userid\' -- to request the specified player to start a new game\nType -- accept_request \'opponent_userid\'-- to accept the game request from another player to start a new game\nType \'move\' followed by a number between 0-8 to send a move\n\n");
    while (1)
	{
		FD_ZERO(&readfds);
		FD_SET(client_socket, &readfds);
		FD_SET(STDIN_FILENO, &readfds);
		int max_sockfd = (client_socket>STDIN_FILENO)?client_socket:STDIN_FILENO;
		activity = select(max_sockfd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
            perror("Select error");
		if (FD_ISSET(STDIN_FILENO, &readfds))
		{
			bzero(buffer, sizeof(buffer));
			fgets(buffer, sizeof(buffer), stdin);
			send(client_socket, buffer, sizeof(buffer), 0);
		}
		if (FD_ISSET(client_socket, &readfds))
		{
			bzero(buffer, sizeof(buffer));
			if((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) == 0)
			{
				printf("You are disconnected from the game.\n");
				break;
			}
			if(strcmp(buffer,"Your turn.\n") == 0)
			{
				//ingame = 1;
				printf("%s\n", buffer);
				printf("Enter your move (0-8): ");
				//scanf("%d", &move);
				fgets(buffer, sizeof(buffer), stdin);
				send(client_socket, &buffer, sizeof(buffer), 0);
				bzero(buffer, sizeof(buffer));
			}
			else if(strcmp(buffer,"Opponent\'s turn.\n") == 0)
			{
				//ingame = 1;
				printf("%s\n", buffer);
				recv(client_socket, &move, sizeof(int), 0);
				printf("%s", buffer);
				bzero(buffer, sizeof(buffer));
			}
			else if(strstr(buffer,"active players") != NULL)
			{
				printf("\n%s\n", buffer);
				bzero(buffer, sizeof(buffer));
			}
			else if(strstr(buffer,"Game request") != NULL)
			{
				printf("%s", buffer);
				bzero(buffer, sizeof(buffer));
				
			}
			else if(strstr(buffer,"accept-request") != NULL)
			{
				printf("%s", buffer);
				bzero(buffer, sizeof(buffer));
				
			}
		}
    }
    close(client_socket);
    return 0;
}
