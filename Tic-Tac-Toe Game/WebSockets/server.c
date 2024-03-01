#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8000
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 2

int client_sockets[MAX_CLIENTS];
char board[3][3];

void initialize_board(char board[3][3])
{
    for (int i = 0; i < 3; i++)
	{
        for (int j = 0; j < 3; j++)
            board[i][j] = ' ';
    }
    printf("\Current Tic-Tac-Toe Board:\n");
    for (int i = 0; i < 3; i++)
	{
        for (int j = 0; j < 3; j++)
            printf("%c ", board[i][j]);
        printf("\n");
    }
}

int is_game_over(char board[3][3], char symbol)
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

int is_draw(char board[3][3])
{
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
			if(board[i][j] == ' ')
				return 0;
	}
	return 1;
}

void display_board(char board[3][3])
{
    for (int i = 0; i < 3; i++)
	{
        for (int j = 0; j < 3; j++)
            printf("%c ", board[i][j]);
        printf("\n");
    }
}

int main()
{
	int server_socket, client_socket, yes = 1;
    struct sockaddr_in server_addr, client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
	{
        perror("Socket creation failed.\n");
        exit(EXIT_FAILURE);
    }
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	printf("Server socket created.\n");
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
	{
        perror("Server bind failed.\n");
        exit(EXIT_FAILURE);
    }
	printf("Server bind success.\n");
    if (listen(server_socket, MAX_CLIENTS) < 0)
	{
        perror("Listen failed.\n");
        exit(EXIT_FAILURE);
    }
    printf("Tic-Tac-Toe WebSocket Server is listening at port %d.\n", PORT);
	while (1)
	{
        client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &client_addr_len);
        if (client_socket == -1)
		{
            perror("Error in accepting the client connection.\n");
            continue;
        }        
    }
    close(server_socket);
    return 0;
}
