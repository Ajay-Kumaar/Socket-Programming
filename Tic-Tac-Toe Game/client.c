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
int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
	{
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
	printf("Client Socket created successfully\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }
	printf("Client connected to the Server successfully\n");
    char board[BOARD_SIZE][BOARD_SIZE];
    int game_status = 0;
    while (1)
	{
        recv(client_socket, board, sizeof(board), 0);
        printf("Current Tic-Tac-Toe Board:\n");
        print_board(board);
        recv(client_socket, &game_status, sizeof(int), 0);
        if (game_status == 1)
		{
            printf("Game over. You win!\n");
            break;
        }
		else if (game_status == -1)
		{
            printf("Game over. It's a tie!\n");
            break;
        }
        int row, col;
        printf("\nEnter row and column numbers (separated by space): ");
        scanf("%d %d", &row, &col);
        send(client_socket, &row, sizeof(int), 0);
        send(client_socket, &col, sizeof(int), 0);
    }
    close(client_socket);
    return 0;
}
