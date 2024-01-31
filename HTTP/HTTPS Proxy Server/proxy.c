#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 8192
#define PROXY_PORT 8040

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("Failed to receive data from client");
        return;
    }
	printf("Received HTTP Request from the Client:\n\n%s\n",buffer);
    buffer[bytes_received] = '\0';
    if (strstr(buffer, "CONNECT") == NULL) {
        printf("Unsupported HTTP method\n");
        return;
    }
    char *host = strtok(buffer + 8, ":");
    char *port_str = strtok(NULL, " ");
    int port = atoi(port_str);
    printf("Connecting to %s:%d...\n\n", host, port);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create server socket");
        return;
    }
	printf("Proxy Server - %s:%d socket created successfully...\n",host, port);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    struct hostent *server_hostent = gethostbyname(host);
    if (server_hostent == NULL) {
        perror("Failed to resolve destination server");
        close(server_socket);
        return;
    }
	printf("%s:%d address resolved...\n",host, port);
    memcpy(&server_addr.sin_addr.s_addr, server_hostent->h_addr, server_hostent->h_length);
    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to destination server");
        close(server_socket);
        return;
    }
    const char *success_response = "HTTP/1.1 200 Connection established\r\n\r\n";
    send(client_socket, success_response, strlen(success_response), 0);
	printf("Connected to %s:%d\n",host, port);
    fd_set read_fds;
    int max_fd = (client_socket > server_socket) ? client_socket : server_socket;
    while (1)
	{
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);
        FD_SET(server_socket, &read_fds);
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Select error");
            break;
        }
        if (FD_ISSET(client_socket, &read_fds)) {
            bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                break;
            }
            send(server_socket, buffer, bytes_received, 0);
        }
        if (FD_ISSET(server_socket, &read_fds)) {
            bytes_received = recv(server_socket, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                break;
            }
            send(client_socket, buffer, bytes_received, 0);
        }
    }
    close(server_socket);
}
int main()
{
    int server_socket, client_socket;
	int yes = 1;
    struct sockaddr_in proxy_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
        perror("Failed to create proxy socket");
        exit(EXIT_FAILURE);
    }
	printf("Client - Proxy Server socket created successfully...\n");
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(0);	
	}
    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    proxy_addr.sin_port = htons(PROXY_PORT);
    if (bind(server_socket, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0) {
        perror("Failed to bind Proxy Server socket");
        exit(EXIT_FAILURE);
    }
	printf("Proxy Server bind success...\n");
    if (listen(server_socket, 10) < 0) {
        perror("Failed to listen for connections");
        exit(EXIT_FAILURE);
    }
    printf("Proxy Server listening on port %d...\n", PROXY_PORT);
    while (1)
	{
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Failed to accept client connection");
            continue;
        }
		printf("\n\nConnection to Client success...\n\n");
        handle_client(client_socket);
		printf("\nClient HTTP Request handled successfully...\n");
		printf("-------------------------------------------");
        close(client_socket);
    }
    close(server_socket);
    return 0;
}
