#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 2
#define BOARD_SIZE 3

void print_board(char board[BOARD_SIZE][BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; ++i)
	{
        for (int j = 0; j < BOARD_SIZE; ++j)
		{
            printf(" %c ", board[i][j]);
            if (j < BOARD_SIZE - 1)
				printf("|");
        }
        printf("\n");
        if (i < BOARD_SIZE - 1)
			printf("-----------\n");
    }
    printf("\n");
}
int check_winner(char board[BOARD_SIZE][BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; ++i)
	{
        if ((board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ') ||
            (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' '))
            return 1;
    }
    if ((board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ') ||
        (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' '))
        return 1;
    for (int i = 0; i < BOARD_SIZE; ++i)
	{
        for (int j = 0; j < BOARD_SIZE; ++j)
		{
            if (board[i][j] == ' ')
                return 0;
        }
    }
    return -1;
}
int is_valid_move(char board[BOARD_SIZE][BOARD_SIZE], int row, int col)
{
    return (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE && board[row][col] == ' ');
}
int main()
{
    int server_socket, client_sockets[MAX_CLIENTS];
	int yes = 1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
	{
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
	printf("\nServer Socket created successfully\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(0);	
	}
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
	printf("Server bind successful\n");
    if (listen(server_socket, MAX_CLIENTS) == -1)
	{
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening at port %d\n\n", PORT);
    for (int i = 0; i < MAX_CLIENTS; ++i)
	{
        client_sockets[i] = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        if (client_sockets[i] == -1)
		{
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }
        printf("Client %d connected\n", i+1);
    }
    char board[BOARD_SIZE][BOARD_SIZE];
    memset(board, ' ', sizeof(board));
    int current_client = 0;
    int game_status = 0;
    while (1)
	{
		for (int i = 0; i < MAX_CLIENTS; ++i)
            send(client_sockets[i], board, sizeof(board), 0);
		for (int i = 0; i < MAX_CLIENTS; ++i)
            send(client_sockets[i], &game_status, sizeof(int), 0);
        int row, col;
        recv(client_sockets[current_client], &row, sizeof(int), 0);
        recv(client_sockets[current_client], &col, sizeof(int), 0);
        if (is_valid_move(board, row, col))
		{
            board[row][col] = (current_client == 0) ? 'X' : 'O';
            game_status = check_winner(board);
            if (game_status != 0)
			{
                for (int i = 0; i < MAX_CLIENTS; ++i)
                    send(client_sockets[i], &game_status, sizeof(int), 0);
                break;
            }
        }
        current_client = 1-current_client;
    }
    for (int i = 0; i < MAX_CLIENTS; ++i)
        close(client_sockets[i]);
    close(server_socket);
    return 0;
}
