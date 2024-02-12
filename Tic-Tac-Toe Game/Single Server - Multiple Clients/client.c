#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1024

char board[3][3];

void print_board(char board[3][3])
{
    printf("\nCurrent board:\n");
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main()
{
	char m;
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
	//char move[1];
	char status[50];
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
	printf("Client socket created.\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
	printf("Client connected to Server.\n");
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    printf("%s\n\n", buffer);
	bzero(buffer, sizeof(buffer));
	bzero(board, sizeof(board));
    while (1)
	{
        recv(client_socket, board, sizeof(board), 0);
        print_board(board);
		while(strcmp(status,"Valid move.\n") != 0)
		{
			bzero(status, sizeof(status));
		    printf("Enter your move (0-8): ");
		    scanf("%c", &m);
		    //sprintf(move, "%c", m);
		    send(client_socket, &m, 1, 0);
		    recv(client_socket, status, sizeof(status), 0);
		}
        printf("%s\n", status);
		recv(client_socket, buffer, sizeof(buffer), 0);
        if (strstr(buffer, "win") || strstr(buffer, "tie"))
            break;
		printf("%s\n", buffer);
		bzero(buffer, sizeof(buffer));
    }
    close(client_socket);
    return 0;
}
