#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmark.h> 

#define PORT 8000
#define BUFFER_SIZE 8192

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

	// Pointer to beggining of file
    fseek(file, 0, SEEK_END);
	// (save) Pointer to end of file
    long length = ftell(file);
	// Pointer to beggining again
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);

    if (buffer) {
        fread(buffer, 1, length, file);
		// Remeber this is syntactic sugar for *(buffer + length)
        buffer[length] = '\0';
    }

    fclose(file);
    return buffer;
}

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    // Read and convert the Markdown file
    char *markdown_content = read_file("README.md");

    if (markdown_content == NULL) {
        close(client_fd);
        return NULL;
    }

	// Get HTML
    cmark_node *document = cmark_parse_document(markdown_content, strlen(markdown_content), CMARK_OPT_DEFAULT);
    char *html_content = cmark_render_html(document, CMARK_OPT_DEFAULT);
    cmark_node_free(document);
    free(markdown_content);

    // Send the HTTP response
    char http_header[BUFFER_SIZE];
    snprintf(http_header, sizeof(http_header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Length: %zu\r\n"
             "Content-Type: text/html\r\n"
             "\r\n%s", strlen(html_content), html_content);

    send(client_fd, http_header, strlen(http_header), 0);

    // Clean up
    free(html_content);
    close(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    int server_fd;

    // Create socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

	// Add options (due to error with port)
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
	    perror("setsockopt");
	    close(server_fd);
	    exit(EXIT_FAILURE);
	}

	// Socket config
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(PORT)
    };

	// Bind socket
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

	// Listen lol
	int MAX_BACKLOG = 100;
    if(listen(server_fd, MAX_BACKLOG) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

		// Accept incoming requests
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

		// Setting aside the value for concurrency
        int *client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;

		// Handle with different threads
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_fd_ptr) != 0) {
            perror("Failed to create thread");
            free(client_fd_ptr);
            close(client_fd);
        } else {
			// Detach thread to avoid memory leaks ! 
            pthread_detach(thread_id);  
        }
    }

    close(server_fd);
    return 0;
}

