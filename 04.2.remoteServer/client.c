#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h> // For fstat
#include <fcntl.h>    // For open
#include "socket_utils.h" 

#define MAX_BUFFER_SIZE 4096 // Define a buffer size for sending/receiving

// Helper function to get file size (optional, using fstat as suggested)
long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    perror("Error getting file size");
    return -1; // Indicate error
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[5];
    int server_port = atoi(argv[6]);
    const char *program = argv[7];
    char args[MAX_BUFFER_SIZE] = ""; // Store concatenated arguments
    const char *input_filename = NULL;
    long file_size = 0;

    // Parse arguments and check for optional file flag
    int arg_idx = 4;
    while (arg_idx < argc) {
        if (strcmp(argv[arg_idx], "-f") == 0) {
            if (arg_idx + 1 < argc) {
                input_filename = argv[arg_idx + 1];
                // Get file size
                file_size = get_file_size(input_filename);
                if (file_size < 0) {
                    exit(EXIT_FAILURE); // Exit if file size cannot be determined
                }
                arg_idx += 2; // Skip -f and filename
            } else {
                fprintf(stderr, "Error: -f requires a filename\n");
                exit(EXIT_FAILURE);
            }
        } else {
            // Add other arguments to the args string
            if (strlen(args) + strlen(argv[arg_idx]) + 2 > MAX_BUFFER_SIZE) {
                 fprintf(stderr, "Arguments string too long\n");
                 exit(EXIT_FAILURE);
            }
            if (strlen(args) > 0) strcat(args, " "); // Add space separator
            strcat(args, argv[arg_idx]);
            arg_idx++;
        }
    }

    //Connect to the server
    int client_socket = tcp_client_socket_init(server_ip, server_port);
    if (client_socket < 0) {
        exit(EXIT_FAILURE);
    }
    printf("Connected to server %s:%d\n", server_ip, server_port);

    //Construct and send the header
    char header[MAX_BUFFER_SIZE];
    // Format: RUN: <program>\n ARGS: <argumentos>\n FILE: <filename>\n DIM: <size>\n \n
    snprintf(header, sizeof(header),
             "RUN: %s\nARGS: %s\nFILE: %s\nDIM: %ld\n\n",
             program,
             args,
             input_filename ? input_filename : "", // Send empty string if no file
             file_size);

    printf("Sending header:\n%s\n", header);

    if (send(client_socket, header, strlen(header), 0) < 0) {
        perror("Error sending header");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    //Send the file content (if applicable)
    if (input_filename != NULL && file_size > 0) {
        printf("Sending file %s (%ld bytes)...\n", input_filename, file_size);
        FILE *file = fopen(input_filename, "rb");
        if (!file) {
            perror("Error opening input file for sending");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        char file_buffer[MAX_BUFFER_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
            if (send(client_socket, file_buffer, bytes_read, 0) < 0) {
                perror("Error sending file content");
                fclose(file);
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }
        fclose(file);
        printf("File content sent.\n");
    }

    //Receive and display the output
    printf("Waiting for server response...\n");
    char response_buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(client_socket, response_buffer, sizeof(response_buffer) - 1, 0)) > 0) {
        response_buffer[bytes_received] = '\0'; // Null-terminate received data
        printf("%s", response_buffer); // Print received data
    }

    if (bytes_received < 0) {
        perror("Error receiving response from server");
    } else {
        printf("\nServer finished sending data.\n"); // Indicates connection closed by server
    }
    
    close(client_socket); // Close the connection
    printf("Connection closed.\n");

    return 0;
}