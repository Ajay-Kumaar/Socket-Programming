#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8000

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
void playGame(int socket,int player_id)
{
    int move,row,col,gameover=0,currentPlayer = 0;
    char board[3][3];
    recv(socket, board, sizeof(board), 0);
    displayBoard(board);
	while(1)
	{
		recv(socket, &currentPlayer , sizeof(int), 0);
		if(currentPlayer == player_id)
		{
			printf("Your turn\n\n");
			while(1)
			{
				printf("Enter your move (0-8): ");
				scanf("%d", &move);
				int row = move / 3;
		    	int col = move % 3;
				if((move<0 || move>8) || (board[row][col] != ' '))
				{
					printf("Invalid move!\n");
					continue;
				}
		        board[row][col] = (currentPlayer == 0) ? 'X' : 'O';
				break;
			}
			send(socket, board, sizeof(board), 0);
		}
		else
		{
			printf("Not your turn, please wait for any further updates.\n");
			recv(socket, board, sizeof(board), 0);
		}
		displayBoard(board);
		recv(socket, &gameover , sizeof(int), 0);
		if(gameover == 1)
		{
			int win_player_id = 0;
			recv(socket,&win_player_id,sizeof(int),0);
			if(player_id == win_player_id)
				printf("\n\nYou won the Game!!!\n\n");
			else
				printf("\n\nBetter Luck next time\n\n");
			return;
		}
		else if(gameover == 2)
			printf("It's a tie!!!\n\n");
    }
}
int main()
{
    int client_socket,player_id = 0;
    struct sockaddr_in server_addr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1)
	{
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
	printf("Client socket created successfully\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }
    printf("Client successfully connected to the Server\n");
	recv(client_socket, &player_id, sizeof(int), 0);
	printf("PlayerID: %d\n\n",player_id);
	while(1)
    	playGame(client_socket,player_id);
    close(client_socket);
    return 0;
}
