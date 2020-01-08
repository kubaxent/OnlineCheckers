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
    bool in_match;
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
player_data* players[PLAYERS_MAX_NUMBER]; //Connected players

//Send/Receive Mutexes
//pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;

char* after_space(char* input) {
    char* starting = input;
    while (*starting != ' ') {
        starting++;
    }
    starting++;
    return starting;
}

player_data* find_player(char *username){
    for(int i = 0; i < PLAYERS_MAX_NUMBER; i++){
        if(players[i]!=NULL){
            printf("%s%s\n",players[i]->username,username);
            if(strcmp(players[i]->username, username)==0)return players[i];
        }
    }
    return NULL;
} 

void * player_session(void *pl_data){
    pthread_detach(pthread_self());

    player_data *p_data = (player_data*)pl_data;
    int index = p_data->id;

    printf("New player session started\n");
    
    //Get the username
    recvfrom(p_data->socket_fd, p_data->username, USERNAME_BUFFER, 0, NULL, NULL);

    printf("Got the username for %s\n", p_data->username);

    char input[MESSAGE_BUFFER];
    char *opponent_name;
    int response;
    while(true){

        response = recvfrom(p_data->socket_fd, input, MESSAGE_BUFFER, 0, NULL, NULL);
        
        //Error/disconnection handling
        if (response == -1) {
            printf("recv() failed\n");
            break;
        }else if (response == 0) {
            printf("%s disconnected\n",p_data->username);
            break;
        }
        if (strncmp(input, "/quit", 5) == 0) {
            printf("Closing connection with %s\n",p_data->username);
            break;
        }
        if (strncmp(input, "/playagainst ", 5) == 0) {
            opponent_name = after_space(input);
            
            printf("Selected opponent: %s\n", opponent_name);
            player_data *opponent = find_player(opponent_name);
            
            if(opponent!=NULL){
                printf("Opponent socket_fd: %d\n", opponent->socket_fd);
                send(p_data->socket_fd, opponent->username, USERNAME_BUFFER, 0);
            }else{
                printf("Could not find opponent.\n");
            }
            //TODO: send play request to opponent

        }
        printf("%s: %s\n",p_data->username, input);
    }

    printf("Shutting player %s session down.\n",p_data->username);
    
    connectedPlayers--;
    close(p_data->socket_fd);
    free(p_data);
    players[index]=NULL; //Apparently good practice
    pthread_exit(NULL); 
}

//Game session thread
void * game_session(void *ga_data){
    pthread_detach(pthread_self());

    game_session_data *g_data = (game_session_data*)ga_data;
    printf("%d\n",g_data->player1->id);

    //recvfrom(cl1_socket_fd, opponent, MESSAGE_BUFFER, 0, NULL, NULL); //Get the opponent's name
    
    //int response;

    while(true){
        //response =
        
    }

    //free(game_data);
    pthread_exit(NULL); 
}

int player_array_empty_index(){
    for(int i = 0; i < PLAYERS_MAX_NUMBER; i++){
        if(players[i]==NULL){
            return i;
        }
    }
    return -1;
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
    struct sockaddr_in address;
    int socket_fd, new_socket_fd;
    char reuse_addr_val = 1;
    
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
            new_socket_fd = accept(socket_fd, NULL, NULL);

            if (new_socket_fd < 0) {
                printf("Failed to connect\n");
                continue;
            }else{
                printf("New accepted\n");
            }

            // Create new player session
            int index = player_array_empty_index(); //Finds the first empty slot in our players array

            players[index] = malloc(sizeof(player_data));
            
            players[index]->socket_fd = new_socket_fd;
            players[index]->id = index;

            pthread_t thread;
            int create_result = pthread_create(&thread, NULL, player_session, (void *)players[index]);

            if (create_result){
                printf("Error while trying to create game session %d\n", create_result);
                free(players[index]);
                players[index]=NULL; //Apparently good practice
                continue;
            }
            
            connectedPlayers++;

        }

    }

    // Close socket and kill thread
    close(socket_fd);
    for(int i = 0; i < PLAYERS_MAX_NUMBER; i++){
        close(players[i]->socket_fd);
        free(players[i]);
        players[i]=NULL; //Apparently good practice
    }
    pthread_exit(NULL);
    return 0;
}