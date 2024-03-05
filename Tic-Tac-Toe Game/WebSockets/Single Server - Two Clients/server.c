#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

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
}

void display_board(char board[3][3])
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

void encode_and_send(const char* str, int length, int client_socket) 
{
	printf("\n\noriginal message: %s\n\n", str);
    char frame[MAX_BUFFER_SIZE];
    frame[0] = 0x81;
    frame[1] = 0x80 | length;
    unsigned char mask_key[4];
    for (int i = 0; i < 4; i++) 
        mask_key[i] = rand() % 256;
    memcpy(frame+2, mask_key, 4);
    for (int i = 0; i < length; i++) 
        frame[i+6] = str[i] ^ mask_key[i%4];
	printf("\n\nencoded frame: %s\n\n", frame);
    send(client_socket, frame, length + 6, 0);
}

int parse_web_socket_frame(char* str, int length, char* output)
{
	printf("\n\nframe from the browser: %s\n\n", str);
    if (length < 6)
        return -1;
    if ((str[0] & 0x80) == 0x80 && (str[0] & 0x0F) == 0x01)
	{
        int payload_length = str[1] & 0x7F;
        int mask_offset = 2;
        int data_offset = mask_offset + 4;
        if (payload_length <= 125)
		{
            for (int i = 0; i < payload_length; i++)
                output[i] = str[data_offset + i] ^ str[mask_offset + i % 4];
            output[payload_length] = '\0';
			printf("\n\nparsed frame: %s\n\n", output);
            return payload_length;
        }
    }
    return -1;
}

void broadcast_move(int row, int col)
{
	char str[MAX_BUFFER_SIZE];
	snprintf(str, sizeof(str), "%d,%d,%c", row, col, board[row][col]);
	for (int k = 0; k < MAX_CLIENTS; k++)
		encode_and_send(str, strlen(str), client_sockets[k]);
}

void handle_player(int client_socket, int player_id)
{
	initialize_board(board);
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0)
	{
		printf("\nWebSocket Handshake request received from the player %d is:\n\n%s\n\n", player_id-1, buffer);
        char* key_start = strstr(buffer, "Sec-WebSocket-Key: ") + 19;
        char* key_end = strchr(key_start, '\r');
        *key_end = '\0';
        char key[128];
        char magic_string[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        strcpy(key, key_start);
        strcat(key, magic_string);
        static unsigned char sha1Hash[SHA_DIGEST_LENGTH];
        SHA1((const unsigned char*)key, strlen(key), sha1Hash);
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
        BIO_free_all(b64);
        const char* response_headers = "HTTP/1.1 101 Switching Protocols\r\n"
                        			   "Upgrade: websocket\r\n"
                        			   "Connection: Upgrade\r\n"
                        			   "Sec-WebSocket-Accept: %.*s\r\n"
                       				   "\r\n";
        char response[MAX_BUFFER_SIZE];
        snprintf(response, sizeof(response), response_headers, length, base64Hash);
        send(client_socket, response, strlen(response), 0);

		char message[MAX_BUFFER_SIZE];
		snprintf(message, sizeof(message), "Player %d", player_id-1);
		encode_and_send(message, strlen(message), client_socket);
    }
	while (1)
	{
        bzero(buffer, sizeof(buffer));
        bytes_received = recv(client_sockets[p], buffer, sizeof(buffer), 0);
        if (bytes_received > 0)
		{
            char message[MAX_BUFFER_SIZE];
            int payload_length = parse_web_socket_frame(buffer, bytes_received, message);
            if (payload_length > 0)
			{
                int row, col;
                sscanf(message, "%d %d", &row, &col);
                printf("\n\nMove received from the player %d is:\nRow:%d\nCol:%d\n\n", p, row, col);
                if( row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ')
				{
                    board[row][col] = (p == 0) ? 'X' : 'O';
                    display_board(board);
                    broadcast_move(row, col);
                    if (is_game_over(board, 'X'))
					{
                        printf("\n\nPlayer 0 wins the game.\n\n");
                        char* str1 = "\nYou won the game!\n";
		            	encode_and_send(str1, strlen(str1), client_sockets[0]);
		            	char* str2 = "\nYou lost the game. Better luck next time!\n";
                        encode_and_send(str2, strlen(str2), client_sockets[1]) ;
                        break;
                    }
					else if (is_game_over(board, 'O'))
					{
                        printf("\n\nPlayer 1 wins the game.\n\n");
                        char* str1 = "\nYou won the game!\n";
		            	encode_and_send(str1, strlen(str1), client_sockets[1]);
		            	char* str2 = "\nYou lost the game. Better luck next time!\n";
                        encode_and_send(str2, strlen(str2), client_sockets[0]) ;
                        break;
                    }
					else if (is_draw(board))
					{
                        printf("\nIt\'s a draw!\n");
                        char* str = "\nIt\'s a draw!\n";
                        for (int k = 0; k < MAX_CLIENTS; k++)
                			encode_and_send(str, strlen(str), client_sockets[k]);
                        break;
                    }
					else
					{
						char* str = "\nGame in progress.\n";
                        for (int k = 0; k < MAX_CLIENTS; k++)
                			encode_and_send(str, strlen(str), client_sockets[k]);
		                if (p == 0)
		                	p++;
		                else
		                	p--;
					}
                }
				else
                    printf("\n\nInvalid move from the player %d.\n\n", p);
            }
			else
                printf("\n\nInvalid WebSocket frame received from the player %d.\n\n", p);
        }
    }
    for (int i = 0; i < MAX_CLIENTS ; i++) 
    {
        close(client_sockets[i]);
        printf("Client Socket %d is closed.\n", i);
    }
    exit(0);
}

void* handle_thread(void* client_socket_ptr)
{
	int client_socket = *((int*)client_socket_ptr);
	if (player_id < MAX_CLIENTS)
	    client_sockets[player_id] = client_socket;
	else
	{
		perror("Exceeded the maximum number of players.\n");
		exit(-1);
	}
	__sync_add_and_fetch(&player_id, 1);
	handle_player(client_socket, player_id);
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
    return 0;
}
