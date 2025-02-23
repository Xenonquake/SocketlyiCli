#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <arpa/inet.h> // For inet_ntop

#define PORT 8080
#define BUFFER_SIZE 1024

static int debug = 0; // Global debug flag

volatile sig_atomic_t interrupted = 0;

void signal_handler(int signal) {
    (void)signal; // Suppress unused parameter warning
    interrupted = 1;
    printf("\nCaught signal, exiting gracefully...\n");
    rl_callback_handler_remove();
}

// Custom debug print function
void debug_print(const char *msg) {
    if (debug) {
        fprintf(stderr, "[DEBUG] %s\n", msg);
    }
}

int main(int argc, char *argv[]) {
    printf("Network Layer Started \n");

    int socket_fd;
    struct sockaddr_in server_address;
    struct hostent *server;
    char *line = nullptr;
    char *hostname = nullptr;

    // Argument parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug = 1;
        } else {
            if (hostname != nullptr) {
                fprintf(stderr, "Error: Only one hostname is expected! [./cliently \"Netowrk\" --debug\n ./cliently --debug \"Network\"\n");
                exit(EXIT_FAILURE);
            }
            hostname = argv[i];
        }
    }

    if (hostname == nullptr) {
        fprintf(stderr, "Error: No hostname provided\n");
        exit(EXIT_FAILURE);
    }

    debug_print("Argument parsing complete");

    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        char errbuf[256];
        strerror_r(errno, errbuf, sizeof(errbuf));
        fprintf(stderr, "Socket creation failed: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    debug_print("Socket created successfully");

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        close(socket_fd); // Clean up socket
        exit(EXIT_FAILURE);
    }

    debug_print("Hostname resolved");

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy(&server_address.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    server_address.sin_port = htons(PORT);

    // Convert the IP address to a string
    char addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_address.sin_addr, addr_str, INET_ADDRSTRLEN);
    char connect_msg[256];
    snprintf(connect_msg, sizeof(connect_msg), "Attempting to connect to %s:%d", addr_str, PORT);
    debug_print(connect_msg);

    // Connect to the server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        char errbuf[256];
        strerror_r(errno, errbuf, sizeof(errbuf));
        fprintf(stderr, "Connection failed: %s\n", errbuf);
        debug_print("Connection attempt failed, closing socket");
        close(socket_fd); // Clean up socket
        exit(EXIT_FAILURE);
    }

    printf("Connected to local server!\n");
    printf("Linked to the local socket.  Enter 'quit' to exit.\n");

    // Signal handling
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    debug_print("Signal handler set up");

    // Setup readline
    rl_bind_key('\t', rl_complete); // Enable tab completion

    while (!interrupted) {
        line = readline("Enter command: ");

        if (line == NULL) {
            // Handle Ctrl+D (EOF)
            printf("\nEOF received, exiting.\n");
            break;
        }

        if (strlen(line) > 0) {
            add_history(line);  // Add command to history

            // Check for quit command
            if (strcmp(line, "quit") == 0) {
                free(line);
                break;
            }

            debug_print("Sending command to server");
            // Send the message to the server
            ssize_t bytes_sent = send(socket_fd, line, strlen(line), 0);
            if (bytes_sent < 0) {
                char errbuf[256];
                strerror_r(errno, errbuf, sizeof(errbuf));
                fprintf(stderr, "Send failed: %s\n", errbuf);
                free(line);
                break;
            }

            char buffer[BUFFER_SIZE] = {0};
            debug_print("Waiting for server response");
            ssize_t bytes_received = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received < 0) {
                char errbuf[256];
                strerror_r(errno, errbuf, sizeof(errbuf));
                fprintf(stderr, "Receive failed: %s\n", errbuf);
                free(line);
                break;
            } else if (bytes_received == 0) {
                printf("Server disconnected\n");
                free(line);
                break;
            }

            printf("Received: %s", buffer);
        }
        free(line);
    }

    close(socket_fd);
    rl_cleanup_after_signal();
    debug_print("Socket closed, exiting");
    return 0;
}
