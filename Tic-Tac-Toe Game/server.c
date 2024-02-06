#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 2

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
int checkWin(char board[3][3])
{
    for(int i=0;i<3;i++)
	{
        if(board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return 1;
        if(board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return 1;
    }
    if(board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return 1;
    if(board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return 1;
    return 0;
}
int checkDraw(char board[3][3])
{
    for(int i=0;i<3;i++)
	{
        for(int j=0;j<3;j++)
		{
            if(board[i][j] == ' ')
                return 0;
        }
    }
    return 1;
}
void playGame(int client_sockets[MAX_CLIENTS])
{
    char board[3][3] = {{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}};
    int currentPlayer = 0,gameover = 0,move;
	for(int i=0;i<MAX_CLIENTS;i++)
		send(client_sockets[i], board, sizeof(board), 0);
    while(!gameover)
	{
		for(int i=0;i<MAX_CLIENTS;i++)
			send(client_sockets[i], &currentPlayer, sizeof(int), 0);
        recv(client_sockets[currentPlayer], board, sizeof(board), 0);
		send(client_sockets[1-currentPlayer], board, sizeof(board), 0);
        if(checkWin(board))
		{
            printf("\n\nWohoo!! Player %d wins!\n", currentPlayer);
            gameover = 1;
        }
		else if(checkDraw(board))
		{
            printf("It's a draw!\n");
            gameover = 2;
        }
		else
			gameover = 0;
		for(int i=0;i<MAX_CLIENTS;i++)
		{
			send(client_sockets[i], &gameover, sizeof(int), 0);
			if(gameover == 1)
				send(client_sockets[i], &currentPlayer, sizeof(int), 0);
		}
        currentPlayer = 1-currentPlayer;
    }
}
int main()
{
	int yes = 1;
    int server_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1)
	{
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(0);	
	}
	printf("Server socket created successfully\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
	printf("Server bind successful\n");
    if(listen(server_socket, MAX_CLIENTS) == -1)
	{
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening at port %d\n\n", PORT);
    for(int i=0;i<MAX_CLIENTS;i++)
	{
        client_sockets[i] = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        printf("Client %d connected.\n", i+1);
    }
	// Creating id for each player and sending it to them
	for(int i=0;i<MAX_CLIENTS;i++)
		send(client_sockets[i], &i, sizeof(int), 0);
	while(1)
	{
		int start = 0;
		printf("\nDo you want to start a new Game (1/0): ");
		scanf("%d",&start);
		if(start)
    		playGame(client_sockets);
	}
    for(int i=0;i<MAX_CLIENTS;i++)
        close(client_sockets[i]);
    close(server_socket);
    return 0;
}
