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
#define PLAYERS_MAX_NUMBER 32

//Player data struct
typedef struct player_data{
    int id;
    int socket_fd;
    //struct sockaddr player_addr;
    char[USERNAME_BUFFER] username;
}player_data;

//game session data
typedef struct game_session_data{
    player_data player1;
    player_data player2;
}game_session_data;

//Global variables
int connectedPlayers = 0; //Number of connected players
bool shutDown = false; //If this is true we shut down the server
player_data[PLAYERS_MAX_NUMBER] players; //connected players

//Send/Receive Mutexes
pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;

void * player_session(){
    pthread_detach(pthread_self());

    connectedPlayers--;
    //close(socket_fd);
    pthread_exit(NULL); 
}

//Game session thread
void * game_session(void *p1_data, void *p2_data){
    pthread_detach(pthread_self());

    //game_session_data *g_data = (game_session_data*)game_data;
    //int cl1_socket_fd = (*g_data).cl_socket_fd;
    //struct sockaddr cl_addr = (*g_data).cl_addr;

    pthread_mutex_lock(&recv_mutex);
        //recvfrom(cl1_socket_fd, opponent, MESSAGE_BUFFER, 0, NULL, NULL); //Get the opponent's name
    pthread_mutex_unlock(&recv_mutex);
    
    int response;

    while(!shutDown){
        pthread_mutex_lock(&recv_mutex);
            //response = 
        pthread_mutex_unlock(&recv_mutex);
    }

    //free(game_data);
    pthread_exit(NULL); 
}


// Get message from stdin and send to client
/*void * send_message(int new_socket_fd, struct sockaddr *cl_addr) {
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
}*/

int main(int argc, char**argv) {
    long port = strtol(argv[1], NULL, 10);
    struct sockaddr_in address, cl_addr;
    int socket_fd, length, response, new_socket_fd;
    char client_address[CLIENT_ADDRESS_LENGTH];
    

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

    while(!shutDown)
    {
        if(connectedPlayers<=PLAYERS_MAX_NUMBER){
            
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
            player_data *p_data = malloc(sizeof(player_data));
            
            p_data->socket_fd = new_socket_fd;
            p_data->id = connectedPlayers;
            //p_data->username = 

            pthread_t thread;
            int create_result = pthread_create(thread, NULL, player_session, (void *)p_data);

            if (create_result){
                printf("Error while trying to create game session %d\n", create_result);
                free(p_data)
                continue;
            }

            players[connectedPlayers] = *p_data;
            connectedPlayers++;


        }

        //pthread_create(&thread[i], NULL, receive, (void *) new_socket_fd);
        // Send message
        //send_message(new_socket_fd, &cl_addr); //For now just testing receiving
    }

    // Close sockets and kill threads
    //close(new_socket_fd); //TODO: Close all new_socket_fd's
    close(socket_fd);
    pthread_exit(NULL);
    return 0;
}