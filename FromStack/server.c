#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "pthread.h"

#define MESSAGE_BUFFER 500
#define USERNAME_BUFFER 10
#define CLIENT_ADDRESS_LENGTH 100
#define GAMES_MAX_NUMBER 32

//Game session struct
typedef struct game_session_data{
    int cl_socket_fd;
    struct sockaddr cl_addr;
} game_session_data;

//Global variables
bool shutDown = false; //If this is true we shut down the server

//Send/Receive Mutexes
pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;

//Game session thread
void * game_session(void *game_data){
    pthread_detach(pthread_self());

    char opponent[USERNAME_BUFFER];
    memset(opponent, 0, USERNAME_BUFFER); // Clear opponent name buffer
    socket_fd = (int) socket;

    pthread_mutex_lock(&recv_mutex);
        recvfrom(socket_fd, opponent, MESSAGE_BUFFER, 0, NULL, NULL); //Get the opponent's name
    pthread_mutex_unlock(&recv_mutex);
    
    game_session_data *g_data = (game_session_data*)game_data;

    int cl_socket_fd = (*g_data).cl_socket_fd;
    struct sockaddr cl_addr = (*g_data).cl_addr;
    

    while(!shutDown){
        pthread_mutex_lock(&recv_mutex);

        pthread_mutex_unlock(&recv_mutex);
    }

    pthread_exit(NULL); 
}


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
    pthread_t thread[GAMES_MAX_NUMBER];

    if (argc < 2) {
        printf("Usage: server port_number\n");
        exit(1);
    }

    memset(&address, 0, sizeof(struct sockaddr));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    //Start the server
    bind(socket_fd, (struct sockaddr *) address, sizeof *address);
    printf("Waiting for first connection...\n");
    listen(socket_fd, 10);

    int i = 0;

    while(!shutDown)
    {
        // Accept connection
        length = sizeof(cl_addr);
        new_socket_fd = accept(socket_fd, (struct sockaddr *) &cl_addr, &length);

        if (new_socket_fd < 0) {
            printf("Failed to connect\n");
            continue;
        }

        inet_ntop(AF_INET, &(cl_addr.sin_addr), client_address, CLIENT_ADDRESS_LENGTH);
        printf("Connected: %s\n", client_address);

        // Create new game thread
        game_session_data *g_data = malloc(sizeof(game_session_data));
        
        g_data->cl_socket_fd = new_socket_fd;
        g_data->cl_addr = cl_addr;

        int create_result = pthread_create(&thread[i], NULL, game_session, (void *)g_data);

        if (create_result){
            printf("Error while trying to create game session %d\n", create_result);
            continue;
        }

        i++;

        //pthread_create(&thread[i], NULL, receive, (void *) new_socket_fd);
        // Send message
        //send_message(new_socket_fd, &cl_addr); //For now just testing receiving
    }

    // Close sockets and kill threads
    close(new_socket_fd); //TODO: Close all new_socket_fd's
    close(socket_fd);
    pthread_exit(NULL);
    return 0;
}