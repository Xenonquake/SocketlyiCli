#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h> // For inet_ntop

#define PORT 8080
#define BUFFER_SIZE 1024

static int debug = 0; // Global debug flag
volatile sig_atomic_t interrupted = 0;

void signal_handler(int signal) {
    (void)signal; // Suppress unused parameter warning
    interrupted = 1;
    printf("\nCaught signal, exiting gracefully...\n");
}

// Custom debug print function
void debug_print(const char *msg) {
    if (debug) {
        fprintf(stderr, "[DEBUG] %s\n", msg);
    }
}

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug = 1;
        }
    }

    debug_print("Server starting");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    pid_t child_pid;

    // Set up signal handling
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    debug_print("Socket created successfully");

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    debug_print("Socket bound to port 8080");

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    debug_print("Server listening on port 8080");

    // Main loop to handle connections
    while (!interrupted) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            if (interrupted) break;
            perror("accept failed");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        char accept_msg[256];
        snprintf(accept_msg, sizeof(accept_msg), "Accepted connection from %s:%d", client_ip, ntohs(address.sin_port));
        debug_print(accept_msg);

        // Process client requests
        while (!interrupted) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes_received = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received < 0) {
                perror("recv failed");
                break;
            } else if (bytes_received == 0) {
                debug_print("Client disconnected");
                break;
            }

            printf("Received: %s\n", buffer);
            debug_print("Command received from client");

            // Fork to execute command
            child_pid = fork();
            if (child_pid == -1) {
                perror("fork failed");
                send(new_socket, "Fork failed", strlen("Fork failed"), 0);
            } else if (child_pid == 0) {
                // Child process
                FILE *fp = popen(buffer, "r");
                char command_output[BUFFER_SIZE] = {0};
                if (fp == NULL) {
                    snprintf(command_output, BUFFER_SIZE, "Failed to execute command\n");
                    send(new_socket, command_output, strlen(command_output), 0);
                    exit(EXIT_FAILURE);
                }
                while (fgets(command_output, BUFFER_SIZE, fp) != NULL) {
                    send(new_socket, command_output, strlen(command_output), 0);
                }
                pclose(fp);
                exit(EXIT_SUCCESS);
            } else {
                // Parent process
                int status;
                waitpid(child_pid, &status, 0);
                debug_print("Child process completed");
                if (WIFEXITED(status)) {
                    char status_msg[256];
                    snprintf(status_msg, sizeof(status_msg), "Child process exited with status %d", WEXITSTATUS(status));
                    debug_print(status_msg);
                } else {
                    debug_print("Child process terminated abnormally");
                }
                const char *response = "Command executed.\n";
                send(new_socket, response, strlen(response), 0);
            }
        }
        close(new_socket);
        debug_print("Client connection closed");
    }

    close(server_fd);
    debug_print("Server shutting down");
    return 0;
}
