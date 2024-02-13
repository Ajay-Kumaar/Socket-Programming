#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int p = 0;

void initialize_board(char board[3][3]);
void print_board(char board[3][3]);
int check_winner(char board[3][3], char symbol);
void *handle_client(void *arg);
int add_client(int server_socket);

typedef struct
{
    int socket;
    char symbol;
	char board[3][3];
}Player;
typedef struct
{
    int socket;
    pthread_t thread_id;
}Client;

char buffer[BUFFER_SIZE];
Client clients[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main()
{
    int server_socket;
	int yes = 1;
    struct sockaddr_in server_addr;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
	printf("Server socket created.\n");
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(0);	
	}
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }
	printf("Server bind success.\n");
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening at port %d.\n\n", PORT);
    if (add_client(server_socket) == -1) {
        return -1;
    }
    close(server_socket);
    return 0;
}
void *handle_client(void *arg)
{
	bzero(buffer, sizeof(buffer));
    int client_index = *((int*)arg);
    int client_socket = clients[client_index].socket;
	//printf("%d\n",client_index);
    Player player;
    player.socket = client_socket;
	//printf("%d\n",player.socket);
    pthread_mutex_lock(&mutex);
	p++;
    if (p % 2 == 0)
        player.symbol = 'O';
    else
        player.symbol = 'X';
	printf("%d\n",p);
	char move;
    sprintf(buffer, "You are player %c\n", player.symbol);
	printf("%s",buffer);
    send(player.socket, buffer, strlen(buffer), 0);
	bzero(buffer, sizeof(buffer));
	initialize_board(player.board);
	pthread_mutex_unlock(&mutex);
	while(p%2 != 0);
		while (1)
		{
			send(player.socket, player.board, sizeof(player.board), 0);
			label:
		    recv(player.socket, &move, 1, 0);
		    //int m = atoi(move);
			int m = move - '0';
		    int row = m / 3;
		    int col = m % 3;
		    if (player.board[row][col] == ' ') {
		        player.board[row][col] = player.symbol;
				send(player.socket, "Valid move.\n", strlen("Valid move.\n"), 0);
		    } else {
		        send(player.socket, "Invalid move, try again\n", strlen("Invalid move, try again\n"), 0);
		        goto label;
		    }
		    if (check_winner(player.board, player.symbol)) {
		        send(player.socket, "Congratulations! You win!\n", strlen("Congratulations! You win!\n"), 0);
		        break;
		    }
		    int is_tie = 1;
		    for (int i = 0; i < 3; ++i) {
		        for (int j = 0; j < 3; ++j) {
		            if (player.board[i][j] == ' ') {
		                is_tie = 0;
		                break;
		            }
		        }
		        if (!is_tie) {
		            break;
		        }
		    }
		    if (is_tie) {
		        send(player.socket, "It's a tie!\n", strlen("It's a tie!\n"), 0);
		        break;
		    }
		    send(player.socket == client_socket ? clients[client_index + 1].socket : clients[client_index - 1].socket, "game in progress...\n", strlen("game in progress...\n"), 0);
		}
   
    clients[client_index].socket = -1;
    close(player.socket);
    return NULL;
}
int check_winner(char board[3][3], char symbol)
{
    for (int i = 0; i < 3; ++i) {
        if ((board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol) ||
            (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol)) {
            return 1;
        }
    }
    if ((board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol) ||
        (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol)) {
        return 1;
    }
    return 0;
}
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
void initialize_board(char board[3][3])
{
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            board[i][j] = ' ';
        }
    }
}
int add_client(int server_socket)
{
	
    for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		int client_socket;
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) == -1) {
		    perror("Accept failed");
		}
		printf("Client connected.\n");

        if (clients[i].socket == 0) {
            clients[i].socket = client_socket;
			int j = i;
            if (pthread_create(&clients[i].thread_id, NULL, handle_client, (void* )&j) != 0) {
                clients[i].socket = -1;
                return -1;
            }
        }
    }
    return 0;
}