#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

#define PORT 8000
#define MAX_CLIENTS 20
#define MAX_BUFFER_SIZE 1024

int n = 0;
int client_sockets[MAX_CLIENTS] = {0};

typedef struct
{
	int id;
	int socket;
	int opp_id;
	int is_available;
	int game_requests[MAX_CLIENTS-1];
    char symbol;
	char board[3][3];
}Player;

Player player[MAX_CLIENTS];

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

void encode_and_send(const char* str, int length, int client_socket) 
{
    char frame[MAX_BUFFER_SIZE];
    frame[0] = 0x81;
    frame[1] = 0x80 | length;
    unsigned char mask_key[4];
    for (int i = 0; i < 4; i++) 
        mask_key[i] = rand() % 256;
    memcpy(frame+2, mask_key, 4);
    for (int i = 0; i < length; i++) 
        frame[i+6] = str[i] ^ mask_key[i%4];
    send(client_socket, frame, length + 6, 0);
}

char is_game_over(char board[3][3], char symbol, int id)
{
	char winning_moves[MAX_BUFFER_SIZE];
	bzero(winning_moves, sizeof(winning_moves));
	if ((board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol))
	{
		snprintf(winning_moves, sizeof(winning_moves), "%d-%d-%d", 0, 4, 8);
		encode_and_send(winning_moves, strlen(winning_moves), player[id].socket);
		encode_and_send(winning_moves, strlen(winning_moves), player[player[id].opp_id].socket);
        return symbol;
	}
	else if ((board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol))
	{
		snprintf(winning_moves, sizeof(winning_moves), "%d-%d-%d", 2, 4, 6);
		encode_and_send(winning_moves, strlen(winning_moves), player[id].socket);
		encode_and_send(winning_moves, strlen(winning_moves), player[player[id].opp_id].socket);
        return symbol;
	}
    for (int i = 0; i < 3; i++)
	{
        if ((board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol))
		{
			snprintf(winning_moves, sizeof(winning_moves), "%d-%d-%d", ((3*i)+(0)), ((3*i)+(1)), ((3*i)+(2)));
			encode_and_send(winning_moves, strlen(winning_moves), player[id].socket);
			encode_and_send(winning_moves, strlen(winning_moves), player[player[id].opp_id].socket);
            return symbol;
		}
		else if ((board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol))
		{
			snprintf(winning_moves, sizeof(winning_moves), "%d-%d-%d", ((3*0)+(i)), ((3*1)+(i)), ((3*2)+(i)));
			encode_and_send(winning_moves, strlen(winning_moves), player[id].socket);
			encode_and_send(winning_moves, strlen(winning_moves), player[player[id].opp_id].socket);
            return symbol;
		}
    }
    return '-';
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

int parse_web_socket_frame(char* str, int length, char* output)
{
    if (length < 6)
        return -1;
    if ((str[0] & 0x80) == 0x80 && (str[0] & 0x0F) == 0x01)//Client > Server - FIN bit is 1(0x80) and it's a text(opcode - 0x01)
	{
        int payload_length = str[1] & 0x7F;
        int mask_offset = 2;
        int data_offset = mask_offset + 4;
        if (payload_length <= 125)
		{
            for (int i = 0; i < payload_length; i++)
                output[i] = str[data_offset + i] ^ str[mask_offset + i % 4];
            output[payload_length] = '\0';
            return payload_length;
        }
    }
    return -1;
}

void send_active_players()
{
	int count;
	char message[MAX_BUFFER_SIZE];
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		count=0;
		bzero(message, sizeof(message));
		sprintf(message, "active_players ");
		for (int j = 0; j < MAX_CLIENTS; j++)
		{
			if (j == i)
				continue;
			if (player[j].is_available == 1 && player[j].socket != 0)
			{
				char ch = j + '0';
				char* chr = &ch;
				*(chr+1) = '\0';
				strcat(message, chr);
				strcat(message, " ");
				count++;
			}
		}
		if (count == 0)
			sprintf(message, "no_active_players");
		encode_and_send(message, strlen(message), client_sockets[i]);	
	}
}

int main()
{
    int server_socket, max_socket, activity, player_socket, yes = 1, move, row, col, count, payload_length, index = 0; 
	ssize_t bytes_received;
	char win_symbol;
	char buffer[MAX_BUFFER_SIZE];
	char message[MAX_BUFFER_SIZE];
	fd_set readfds;
	struct sockaddr_in server_addr;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
	{
        perror("Socket creation failed.\n");
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
        perror("Bind failed.\n");
        exit(EXIT_FAILURE);
    }
	printf("Server bind success.\n");
    if (listen(server_socket, MAX_CLIENTS) < 0)
	{
        perror("Listen failed.\n");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening at port %d.\n", PORT);
    while (1)
	{
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_socket = server_socket;
        for (int i = 0; i < MAX_CLIENTS; i++)
		{
            player_socket = client_sockets[i];
            if (player_socket > 0)
                FD_SET(player_socket, &readfds);
            if (player_socket > max_socket)
                max_socket = player_socket;
        }
        activity = select(max_socket + 1, &readfds, NULL, NULL, NULL);
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
                perror("Client connection error.\n");
                exit(EXIT_FAILURE);
            }
			bzero(buffer, sizeof(buffer));
			bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
			if (bytes_received > 0)
			{
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(client_sockets[i] == 0)
					{
						n = i;
						printf("\nPlayer %d connected...\n\nDetails of Player %d:\n\n -->Socket File Descriptor: %d\n -->IPv4 Address: %s\n -->Port number: %d\n\n", n, n, client_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
						player[i].id = i;
						player[i].socket = client_socket;
						player[i].is_available = 1;
						client_sockets[i] = client_socket;
						for(int j = 0; j < MAX_CLIENTS-1; j++)
							player[i].game_requests[j] = -1;
						break;
					}
				}
				printf("\nWebSocket Handshake request received from the player %d is:\n\n%s\n\n", n, buffer);
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

				bzero(message, sizeof(message));
				snprintf(message, sizeof(message), "Player %d", n);
				encode_and_send(message, strlen(message), client_socket);
			}
        }

		send_active_players(); //

        for (int i = 0; i < MAX_CLIENTS; i++)
		{
            player_socket = client_sockets[i];
            if (FD_ISSET(player_socket, &readfds))
			{
				bzero(buffer, sizeof(buffer));
                bytes_received = recv(player_socket, buffer, sizeof(buffer), 0);
				if (bytes_received == 0)
				{
                   	printf("\nPlayer %d disconnected from the game...\n", i);
                    close(player_socket);
                    client_sockets[i] = 0;
					player[i].socket = 0;
					send_active_players(); //
                }
				else
				{
                    buffer[bytes_received] = '\0';
					bzero(message, sizeof(message));
					payload_length = parse_web_socket_frame(buffer, bytes_received, message);
            		if (payload_length > 0)
					{
						if (strstr(message, "game_request") != NULL)
						{
							char* ptr = strstr(message, " ");
							ptr+=1;
							int opp = atoi(ptr);
							int already_requested = 0;
							for(int j = 0; j < MAX_CLIENTS-1; j++)
							{
								if(player[opp].game_requests[j] == i)
								{
									already_requested = 1;
									break;
								}
							}
							if(already_requested == 0)
							{
								player[opp].game_requests[index++] = i;
								bzero(message, sizeof(message));
								sprintf(message, "Game request from playerID %d", i);
								encode_and_send(message, strlen(message), client_sockets[opp]);
								bzero(message, sizeof(message));
							}
						}
						else if (strstr(message, "accept_request") != NULL)
						{
							char* ptr = strstr(message, " ");
							ptr+=1;
							int curr = atoi(ptr);
							int opp =  i;
							player[curr].symbol = 'X';
							player[opp].symbol = 'O';
							player[curr].opp_id = opp;
							player[opp].opp_id = curr;
							player[curr].is_available = 0;
							player[opp].is_available = 0;
							send_active_players(); //
							bzero(message, sizeof(message));
							sprintf(message, "Game request accepted");
							encode_and_send(message, strlen(message), player[curr].socket);
							bzero(message, sizeof(message));
							sprintf(message, "Game request accepted");
							encode_and_send(message, strlen(message), player[opp].socket);
							bzero(message, sizeof(message));
							sprintf(message, "Symbol %c", player[curr].symbol);
							encode_and_send(message, strlen(message), player[curr].socket);
							bzero(message, sizeof(message));
							sprintf(message, "Symbol %c", player[opp].symbol);
							encode_and_send(message, strlen(message), player[opp].socket);
							bzero(message, sizeof(message));
							sprintf(message, "Your turn");
							encode_and_send(message, strlen(message), player[curr].socket);
							bzero(message, sizeof(message));
							sprintf(message, "Opponent's turn");
							encode_and_send(message, strlen(message), player[opp].socket);
							initialize_board(player[curr].board);
							initialize_board(player[opp].board);
							bzero(message, sizeof(message));
						}
						else if(strstr(message, "current") != NULL)
						{
							/*for(int j = 0; j < MAX_CLIENTS; j++)
							{
								bzero(message, sizeof(message));
								sprintf(message, "current_list_of_player_requests ");
								for (int k = 0; k < MAX_CLIENTS; k++)
								{
									if (k == j)
										continue;
									if (player[k].is_available == 1 && player[k].socket != 0)
									{
										char ch = k + '0';
										char* chr = &ch;
										*(chr+1) = '\0';
										strcat(message, chr);
										strcat(message, " ");
									}
								}
								encode_and_send(message, strlen(message), client_sockets[j]);	
							}*/
							char* message_copy;
							strcpy(message_copy, message);
							char* current_list[MAX_CLIENTS-1];
							char* str = &message[8];
							for(int j = 8; j < strlen(message); j++)
							{
								if(message_copy[j] == ' ')
								{
									*str
								}
								str += 1;
							}
						}
						else if (strstr(message, " move") != NULL)
						{
                			sscanf(message, "%d %d", &row, &col);
							if(row >= 0 && row < 3 && col >= 0 && col < 3 && player[i].board[row][col] == ' ')
							{
						        player[i].board[row][col] = player[i].symbol;
								player[player[i].opp_id].board[row][col] = player[i].symbol;
								bzero(message, sizeof(message));
								snprintf(message, sizeof(message), "%d,%d,%c", row, col, player[i].board[row][col]);
								encode_and_send(message, strlen(message), player[i].socket);
								encode_and_send(message, strlen(message), player[player[i].opp_id].socket);
						        if ((win_symbol = is_game_over(player[i].board, player[i].symbol, player[i].id)) == player[i].symbol)
								{
						            char* str1 = "You won the game!";
									encode_and_send(str1, strlen(str1), player[i].socket);
									char* str2 = "You lost the game. Better luck next time!";
						            encode_and_send(str2, strlen(str2), player[player[i].opp_id].socket);
									printf("\nPlayer %d disconnected from the game...\n", player[i].id);
									printf("\nPlayer %d disconnected from the game...\n", player[i].opp_id);
								    close(player[i].socket);
									close(player[player[i].opp_id].socket);
								    client_sockets[player[i].id] = 0;
									client_sockets[player[i].opp_id] = 0;
									player[i].socket = 0;
									player[player[i].opp_id].socket = 0;
									send_active_players();
									break;
								}
								else if (win_symbol == player[player[i].opp_id].symbol)
								{
						            char* str1 = "You won the game!";
									encode_and_send(str1, strlen(str1), player[player[i].opp_id].socket);
									char* str2 = "You lost the game. Better luck next time!";
						            encode_and_send(str2, strlen(str2), player[i].socket);
									printf("\nPlayer %d disconnected from the game...\n", player[i].id);
									printf("\nPlayer %d disconnected from the game...\n", player[i].opp_id);
								    close(player[i].socket);
									close(player[player[i].opp_id].socket);
								    client_sockets[player[i].id] = 0;
									client_sockets[player[i].opp_id] = 0;
									player[i].socket = 0;
									player[player[i].opp_id].socket = 0;
									send_active_players();
						            break;
						        }
								else if (is_draw(player[i].board))
								{
						            char* str = "It's a draw!";
						            encode_and_send(str, strlen(str), player[i].socket);
									encode_and_send(str, strlen(str), player[player[i].opp_id].socket);
									printf("\nPlayer %d disconnected from the game...\n", player[i].id);
									printf("\nPlayer %d disconnected from the game...\n", player[i].opp_id);
								    close(player[i].socket);
									close(player[player[i].opp_id].socket);
								    client_sockets[player[i].id] = 0;
									client_sockets[player[i].opp_id] = 0;
									player[i].socket = 0;
									player[player[i].opp_id].socket = 0;
									send_active_players();
						            break;
						        }
								else
								{
									char* str = "Game in progress.";
						            encode_and_send(str, strlen(str), player[i].socket);
									encode_and_send(str, strlen(str), player[player[i].opp_id].socket);
									bzero(message, sizeof(message));
									snprintf(message, sizeof(message), "Your turn");
									encode_and_send(message, strlen(message), player[player[i].opp_id].socket);
									bzero(message, sizeof(message));
									snprintf(message, sizeof(message), "Opponent's turn");
									encode_and_send(message, strlen(message), player[i].socket);
								}
						    }
							else
						        printf("\n\nInvalid move from the player %d", i);
						}
					}
					else
                		printf("\n\nInvalid WebSocket frame received from the player %d", i);
                }
            }
        }
    }
	close(server_socket);
    return 0;
}
