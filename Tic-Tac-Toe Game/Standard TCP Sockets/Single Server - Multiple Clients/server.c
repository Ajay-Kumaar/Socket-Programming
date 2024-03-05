#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8000
#define MAX_CLIENTS 20
#define MAX_BUFFER_SIZE 65536

typedef struct
{
	int id;
	int opp_id;
    int socket;
    char symbol;
	char board[3][3];
}Player;

int client_sockets[MAX_CLIENTS] = {0};
int active_users[MAX_CLIENTS] = {0};
int n = 0;

void initialize_board(char board[3][3])
{
	for (int i = 0; i < 3; i++)
	{
        for (int j = 0; j < 3; j++)
            board[i][j] = ' ';
    }
}

void display_board(char board[3][3])
{
	printf("\nCurrent Tic-Tac-Toe board:\n\n");
	printf("-----------\n");
    for (int i = 0; i < 3; i++)
	{
        for (int j = 0; j < 3; j++)
		{
            printf(" %c ", board[i][j]);
            if (j < 2)
			{
				printf("|");
			}
        }
        printf("\n");
        if (i < 2)
			printf("-----------\n");
    }
	printf("-----------\n");
    printf("\n");
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

int is_valid_move(char board[3][3], int row, int col)
{
	if (row<0 || row>2 || col<0 || col>2)
		return 0;
    else if (board[row][col] == ' ')
    	return 1;
    return 0;
}

int main()
{
    int server_socket, max_sockfd, activity, sd, yes = 1, move, row, col;
    struct sockaddr_in server_addr;
    fd_set readfds;
    Player player[MAX_CLIENTS];
	char buffer[MAX_BUFFER_SIZE];
	char msg[MAX_BUFFER_SIZE];
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
            printf("\nPlayer %d connected...\n\nDetails of Player %d:\n\n -->Socket File Descriptor: %d\n -->IPv4 Address: %s\n -->Port number: %d\n\n", n, n, client_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			player[n].id = n;
			player[n].socket = client_socket;
			active_users[n] = 1;
			n++;
            for (int i = 0; i < MAX_CLIENTS; i++)
			{
                if (client_sockets[i] == 0)
				{
                    client_sockets[i] = client_socket;
                    break;
                }
            }
        }
        for (int i = 0; i < MAX_CLIENTS; i++)
		{
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds))
			{
				bzero(buffer, sizeof(buffer));
                ssize_t bytes_received = recv(sd, buffer, sizeof(buffer), 0);
				if (bytes_received == 0)
				{
                   	printf("\nPlayer %d disconnected from the game...\n", i);
                    close(sd);
                    client_sockets[i] = 0;
					n--;
                }
				else
				{
                    buffer[bytes_received] = '\0';
                    int count;
					bzero(msg, sizeof(msg));
                    if (strstr(buffer, "active_players") != NULL)
					{
						count = 0;
						sprintf(msg, "List of active players:\n");
						for (int j=0; j<MAX_CLIENTS; j++)
						{
							if (j == i)
								continue;
							if (active_users[j] == 1)
							{
								strcat(msg, "\n");
								char ch = j + '0';
								char* chr = &ch;
								strcat(msg, chr);
								count++;
							}
						}
						strcat(msg, "\n");
						if (count == 0)
							sprintf(msg, "There are no active players to start a new game.\n");
						send(sd, msg, sizeof(msg), 0);
					}
					else if (strstr(buffer, "game_request") != NULL)
					{
						char* ptr = strstr(buffer, " ");
						ptr+=1;
						int opp = atoi(ptr);
						bzero(msg, sizeof(msg));
						sprintf(msg, "Game request from playerID %d.\n",i);
						send(client_sockets[opp], msg, strlen(msg), 0);
						bzero(msg, sizeof(msg));
					}
					else if (strstr(buffer, "accept_request") != NULL)
					{
						char* ptr = strstr(buffer, " ");
						ptr+=1;
						int curr = atoi(ptr);
						int opp =  i;
						player[curr].symbol = 'X';
						player[opp].symbol = 'O';
						player[curr].opp_id = opp;
						player[opp].opp_id = curr;
						active_users[curr] = 0;
						active_users[opp] = 0;
						bzero(msg, sizeof(msg));
						sprintf(msg, "\nGame request accepted.\n");
						strcat(msg, "Your symbol: ");
						char* s1 = &player[curr].symbol;
						strcat(msg, s1);
						strcat(msg, "\nYour turn.");
						send(player[curr].socket, msg, sizeof(msg), 0);
						bzero(msg, sizeof(msg));
						sprintf(msg, "\nYour symbol: ");
						char* s2 = &player[opp].symbol;
						strcat(msg, s2);
						strcat(msg, "\nOpponent\'s turn. Please wait for some time...\n\n");
						send(player[opp].socket, msg, sizeof(msg), 0);
						initialize_board(player[curr].board);
						initialize_board(player[opp].board);
						bzero(msg, sizeof(msg));
					}
					else if (strstr(buffer, "move ") != NULL)
					{
						char* ptr = strstr(buffer, " ");
						ptr+=1;
						move = atoi(ptr);
						row = move/3;
						col = move%3;
						if (is_valid_move(player[i].board, row, col))
						{
							player[i].board[row][col] = player[i].symbol;
							player[player[i].opp_id].board[row][col] = player[i].symbol;
							if (is_game_over(player[i].board, player[i].symbol))
							{
								bzero(msg, sizeof(msg));
								sprintf(msg, "winning_move %d %c", move, player[i].symbol);
								send(player[i].socket, msg, sizeof(msg), 0);
								send(player[player[i].opp_id].socket, msg, sizeof(msg), 0);

								bzero(msg, sizeof(msg));
								sprintf(msg, "\n\nCongrats %c! You have won the match.\n\n", player[i].symbol);
								send(player[i].socket, msg, sizeof(msg), 0);
								bzero(msg, sizeof(msg));
								sprintf(msg, "\n\nSorry %c, you have lost the match. Better luck next time!\n\n", player[player[i].opp_id].symbol);
								send(player[player[i].opp_id].socket, msg, sizeof(msg), 0);
								bzero(msg, sizeof(msg));
							}
							else if (is_draw(player[i].board))
							{
								bzero(msg, sizeof(msg));
								sprintf(msg, "draw_move %d %c", move, player[i].symbol);
								send(player[i].socket, msg, sizeof(msg), 0);
								send(player[player[i].opp_id].socket, msg, sizeof(msg), 0);
	
								send(player[i].socket, "\n\nGame has ended in draw.\n\n", sizeof("\n\nGame has ended in draw.\n\n"), 0);
								send(player[player[i].opp_id].socket, "\n\nGame has ended in draw.\n\n", sizeof("\n\nGame has ended in draw.\n\n"), 0);
								bzero(msg, sizeof(msg));
							}
							else
							{			
								send(player[i].socket, "Valid move. It\'s your opponent\'s turn.\n", sizeof("Valid move.It\'s your opponent\'s turn.\n"), 0);
								bzero(msg, sizeof(msg));
								sprintf(msg, "\nOpponent\'s move was: %d. Now it\'s your turn.", move);
								send(player[player[i].opp_id].socket, msg, sizeof(msg), 0);
								bzero(msg, sizeof(msg));
							}
						}
						else
						{
							send(player[i].socket, "Invalid move.\n", sizeof("Invalid move.\n"), 0);
						}
					}
                }
            }
        }
    }
	close(server_socket);
    return 0;
}
