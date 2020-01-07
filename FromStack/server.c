#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "unistd.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "pthread.h"
#include "arpa/inet.h"

#define MESSAGE_BUFFER 500
#define USERNAME_BUFFER 16
#define CLIENT_ADDRESS_LENGTH 100
#define PLAYERS_MAX_NUMBER 32

//Player data struct
typedef struct player_data{
    int id;
    int socket_fd;
    //struct sockaddr player_addr;
    char username[USERNAME_BUFFER];
}player_data;

//game session data
typedef struct game_session_data{
    player_data *player1;
    player_data *player2;
}game_session_data;

//Global variables
int connectedPlayers = 0; //Number of connected players
bool shutDown = false; //If this is true we shut down the server
player_data* players[PLAYERS_MAX_NUMBER]; //connected players

//Send/Receive Mutexes
pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;

void * player_session(void *pl_data){
    pthread_detach(pthread_self());

    player_data *p_data = (player_data*)pl_data;

    printf("Session started\n");
    
    //Get the username
    pthread_mutex_lock(&recv_mutex);
        recvfrom(p_data->socket_fd, p_data->username, USERNAME_BUFFER, 0, NULL, NULL);
    pthread_mutex_unlock(&recv_mutex);

    printf("Got the username for %s\n", p_data->username);

    char input[MESSAGE_BUFFER];

    bool quit = false;
    while(!quit){
        printf("Before mutex for %s\n", p_data->username);
        pthread_mutex_lock(&recv_mutex);
            recvfrom(p_data->socket_fd, input, MESSAGE_BUFFER, 0, NULL, NULL);
            printf("In mutex for %s\n", p_data->username);
        pthread_mutex_unlock(&recv_mutex);
        printf("Out of mutex for %s\n", p_data->username);
        if (strncmp(input, "/quit", 5) == 0) {
            printf("Closing connection with %s\n",p_data->username);
            quit = true;
        }
        printf("%s: %s\n",p_data->username, input);
    }

    printf("Shutting player %s session down.\n",p_data->username);
    connectedPlayers--;
    close(p_data->socket_fd);
    free(p_data);
    pthread_exit(NULL); 
}

//Game session thread
void * game_session(void *ga_data){
    pthread_detach(pthread_self());

    game_session_data *g_data = (game_session_data*)ga_data;
    printf("%d\n",g_data->player1->id);

    pthread_mutex_lock(&recv_mutex);
        //recvfrom(cl1_socket_fd, opponent, MESSAGE_BUFFER, 0, NULL, NULL); //Get the opponent's name
    pthread_mutex_unlock(&recv_mutex);
    
    //int response;

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
    struct sockaddr_in address;//, cl_addr;
    int socket_fd, new_socket_fd;
    char reuse_addr_val = 1;
    //char client_address[CLIENT_ADDRESS_LENGTH];
    
    if (argc < 2) {
        printf("Usage: server port_number\n");
        exit(1);
    }

    memset(&address, 0, sizeof(struct sockaddr));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

    //Start the server
    bind(socket_fd, (struct sockaddr *) &address, sizeof address);
    printf("Waiting for first connection...\n");
    listen(socket_fd, 20);

    while(!shutDown)
    {
        if(connectedPlayers<=PLAYERS_MAX_NUMBER){
            
            // Accept connection
            //socklen_t length = sizeof(struct sockaddr_in);
            new_socket_fd = accept(socket_fd, NULL, NULL);

            if (new_socket_fd < 0) {
                printf("Failed to connect\n");
                continue;
            }else{
                printf("New accepted\n");
            }

            //(AF_INET, &(cl_addr.sin_addr), client_address, CLIENT_ADDRESS_LENGTH);
            //printf("Connected: %s\n", client_address);

            // Create new player session
            players[connectedPlayers] = malloc(sizeof(player_data));
            
            players[connectedPlayers]->socket_fd = new_socket_fd;
            players[connectedPlayers]->id = connectedPlayers;

            pthread_t thread;
            int create_result = pthread_create(&thread, NULL, player_session, (void *)players[connectedPlayers]);

            if (create_result){
                printf("Error while trying to create game session %d\n", create_result);
                free(players[connectedPlayers]);
                continue;
            }
            
            connectedPlayers++;

        }

    }

    // Close socket and kill thread
    close(socket_fd);
    pthread_exit(NULL);
    return 0;
}