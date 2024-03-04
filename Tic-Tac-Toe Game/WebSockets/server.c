#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#define PORT 8000
#define MAX_CLIENTS 2
#define MAX_BUFFER_SIZE 1024

int p = 0;
int client_sockets[MAX_CLIENTS];
char board[3][3];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t player_id = 0;

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

void handle_player(int client_socket, int player_id)
{
	initialize_board(board);
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer));
    printf("\nWebSocket Handshake request received from the player %d is:\n\n%s\n\n", player_id, buffer);
    if (bytes_received > 0)
	{
        char* key_start = strstr(buffer, "Sec-WebSocket-Key: ") + 19;
        char* key_end = strchr(key_start, '\r');
        *key_end = '\0';
        char key[128];
        char magic_string[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        strcpy(key, key_start);
        strcat(key, magic_string);
        printf("\nThe Concatenated Key is: %s.\n\n", key);
        static unsigned char sha1Hash[SHA_DIGEST_LENGTH];
        SHA1((const unsigned char *)concatenatedKey, strlen(concatenatedKey), sha1Hash);
        printf("\n\nSHA1 is: %s.\n\n", sha1Hash);
        BIO *bmem = BIO_new(BIO_s_mem());
        BIO *b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, bmem);
        BIO_write(b64, sha1Hash, SHA_DIGEST_LENGTH);
        BIO_flush(b64);
        char* base64Hash = (char *)malloc(40);
        BUF_MEM *bptr;
        BIO_get_mem_ptr(b64, &bptr);
        strcpy(base64Hash, bptr->data);
        int length = strlen(base64Hash);
        if (length > 0 && base64Hash[length-1] == '\n')
            length--;
        printf("\n\nThe base is: %s\n\n", base64Hash);
        BIO_free_all(b64);
        const char* response_headers = "HTTP/1.1 101 Switching Protocols\r\n"
                        			   "Upgrade: websocket\r\n"
                        			   "Connection: Upgrade\r\n"
                        			   "Sec-WebSocket-Accept: %.*s\r\n"
                       				   "\r\n";
        char response[256];
        snprintf(response, sizeof(response), response_headers, length, base64Hash);
        write(client_socket, response, strlen(response));
    }
}

void* handle_thread(void* client_socket_ptr)
{
	printf("\n\nIn handle_thread function.\n\n");
	printf("\n\n%ld is the player number.\n\n",player_id);
	int client_socket = *((int*)client_socket_ptr);
	if (player_id < MAX_CLIENTS)
	    client_sockets[player_id] = client_socket;
	else
	{
		perror("Exceeded the maximum number of players.\n");
		exit(-1);
	}
	pthread_t currentPlayerNumber = __sync_add_and_fetch(&player_id, 1);
	handle_player(client_socket, player_id);
	//handle_player(client_socket, currentPlayerNumber);
	for (int i = 0; i < player_id; i++) 
	{
	     close(client_sockets[i]);
	     printf("\n\nClient Socket %d is closed.\n\n", i);
	}
	free(client_socket_ptr);
	pthread_exit(NULL);
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
		pthread_t thread_id;
        int* client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;
        if (pthread_create(&thread_id, NULL, handle_thread, client_socket_ptr) != 0)
		{
            perror("Error in creating the thread.\n");
            close(client_socket);
            free(client_socket_ptr);
        }    
    }
    close(server_socket);
    return 0;
}
