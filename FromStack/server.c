#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "pthread.h"

#define MESSAGE_BUFFER 500
#define CLIENT_ADDRESS_LENGTH 100

bool shutDown = false;

// Start server
void * start_server(int socket_fd, struct sockaddr_in *address) {
    bind(socket_fd, (struct sockaddr *) address, sizeof *address);
    printf("Waiting for connection...\n");
    listen(socket_fd, 10);
}

//void * handle_connection(int connection_socket_descriptor){
//
//}

// Get message from stdin and send to client
void * send_message(int new_socket_fd, struct sockaddr *cl_addr) {
    char message[MESSAGE_BUFFER];
    while (fgets(message, MESSAGE_BUFFER, stdin) != NULL) {
        if (strncmp(message, "/quit", 5) == 0) {
            printf("Closing connection...\n");
            shutDown = true;
            exit(0);
        }
        sendto(new_socket_fd, message, MESSAGE_BUFFER, 0, (struct sockaddr *) &cl_addr, sizeof cl_addr);
    }
}

void * receive(void * socket) {
    int socket_fd, response;
    char message[MESSAGE_BUFFER];
    memset(message, 0, MESSAGE_BUFFER); // Clear message buffer
    socket_fd = (int) socket;

    // Print received message
    while(true) {
        response = recvfrom(socket_fd, message, MESSAGE_BUFFER, 0, NULL, NULL);
        if (response) {
            //printf("Socket: %n\n", socket_fd)
            printf("%s %d", message, socket_fd);
        }
    }
}

int main(int argc, char**argv) {
    long port = strtol(argv[1], NULL, 10);
    struct sockaddr_in address, cl_addr;
    int socket_fd, length, response, new_socket_fd;
    char client_address[CLIENT_ADDRESS_LENGTH];
    pthread_t thread[32];

    if (argc < 2) {
        printf("Usage: server port_number\n");
        exit(1);
    }

    memset(&address, 0, sizeof(struct sockaddr));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    start_server(socket_fd, &address);

    int i = 0;

    while(!shutDown)
    {
        // Accept connection
        length = sizeof(cl_addr);
        new_socket_fd = accept(socket_fd, (struct sockaddr *) &cl_addr, &length);

        if (new_socket_fd < 0) {
            printf("Failed to connect\n");
            //exit(1);
        }

        inet_ntop(AF_INET, &(cl_addr.sin_addr), client_address, CLIENT_ADDRESS_LENGTH);
        printf("Connected: %s\n", client_address);

        //handle_connection(new_socket_fd);

        // Create new thread to receive messages
        pthread_create(&thread[i], NULL, receive, (void *) new_socket_fd);
        i++;

        printf("%d\n",i);

        // Send message
        //send_message(new_socket_fd, &cl_addr); //for now just testing receiving
    }

    // Close sockets and kill thread
    close(new_socket_fd);
    close(socket_fd);
    pthread_exit(NULL);
    return 0;
}