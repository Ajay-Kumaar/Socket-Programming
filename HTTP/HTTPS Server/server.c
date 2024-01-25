#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 4433
#define CERT_FILE "server.crt"
#define KEY_FILE "server.key"
#define FILE_DIR "/var/www/html"
#define MAX_BUFFER_SIZE 1024

void send_http_response(SSL* ssl, const char* response)
{
    SSL_write(ssl,response,strlen(response));
}
void handle_http_request(SSL* ssl, const char* request)
{
	FILE* file;
    char method[10], resource[MAX_BUFFER_SIZE];
    sscanf(request, "%s %s", method, resource);
    char file_path[sizeof(FILE_DIR) + MAX_BUFFER_SIZE];
    snprintf(file_path, sizeof(file_path), "%s%s", FILE_DIR, resource);
	if(strcmp(resource,"/") == 0)
		file = fopen("/var/www/html/index.nginx-debian.html", "r");
	else	
    	file = fopen(file_path, "r");
    if (file != NULL)
	{
        const char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 1024\r\n\r\n";
        send_http_response(ssl,response_header);
        char file_buffer[2*MAX_BUFFER_SIZE];
        size_t bytesRead;
        while ((bytesRead = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0)
            SSL_write(ssl, file_buffer, bytesRead);
        fclose(file);
    }
	else
	{
        const char* response_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n404 - Not Found\n";
        send_http_response(ssl,response_header);
    }
}
void init_openssl()
{
    SSL_load_error_strings();
    SSL_library_init();
}
SSL_CTX* create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    method = SSLv23_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx)
	{
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}
void configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);
    if (SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	{
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}
void handle_client(SSL* ssl)
{
    char request[MAX_BUFFER_SIZE];
    if(SSL_read(ssl, request, sizeof(request)) <= 0)
	{
		handle_http_request(ssl, request);
		return;
	}
    printf("\nReceived HTTPS request from the client: %s\n", request);
	handle_http_request(ssl, request);
}
int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    SSL_CTX *ctx;
    init_openssl();
    ctx = create_context();
    configure_context(ctx);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
	{
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }
	printf("Socket opened successfully...\n");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }
	printf("Server bind success...\n");
    if (listen(server_fd, 10) == -1)
	{
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }
	printf("Server listening at port %d\n",PORT);
    while (1)
	{
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1)
		{
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }
		printf("Connection with client success...\n\n");
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);
        if (SSL_accept(ssl) <= 0)
		{
            ERR_print_errors_fp(stderr);
			exit (EXIT_FAILURE);
        }
		else
		{
            handle_client(ssl);
            SSL_shutdown(ssl);
        }
        SSL_free(ssl);
        close(client_fd);
    }
    close(server_fd);
    SSL_CTX_free(ctx);
    return 0;
}
