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
            if(strcmp(players[i]->username, username)==0)return players[i];
        }
    }
    return NULL;
}

//Game session thread
void * game_session(void *ga_data){
    pthread_detach(pthread_self());

    game_session_data *g_data = (game_session_data*)ga_data;

    player_data *p1 = g_data->player1;
    player_data *p2 = g_data->player2;

    char input1[MESSAGE_BUFFER];
    char input2[MESSAGE_BUFFER];
    char message[MESSAGE_BUFFER];

    strcpy(message, "You are now in a match against ");
    strcat(message, p2->username);
    send(p1->socket_fd, message, MESSAGE_BUFFER, 0);

    strcpy(message, "You are now in a match against ");
    strcat(message, p1->username);
    send(p2->socket_fd, message, MESSAGE_BUFFER, 0);

    
    int response;
    bool turn = rand() & 1; //So that a random player starts.
    player_data *current_player;
    player_data *current_opponent;
    while(true){
        
        current_player = (turn)?p1:p2;
        current_opponent = (turn)?p2:p1; 

        response = recv(current_player->socket_fd, input1, MESSAGE_BUFFER, 0);
        if (response <= 0) {
            printf("Game session end due to recv() error.\n");
            break;
        }

        response = recv(current_opponent->socket_fd, input2, MESSAGE_BUFFER, 0);
        if (response <= 0) {
            printf("Game session end due to recv() error.\n");
            break;
        }

        //Commands handling
        if (strncmp(input1, "/end", 4) == 0) {
            printf("Ending game.\n");
            break;
        }
        if (strncmp(input2, "/end", 4) == 0) {
            printf("Ending game.\n");
            break;
        }

        turn=!turn;

    }

    g_data->player1->in_match = false;
    g_data->player2->in_match = false;
    free(g_data);
    pthread_exit(NULL); 
} 

void * player_session(void *pl_data){
    pthread_detach(pthread_self());

    player_data *p_data = (player_data*)pl_data;
    int index = p_data->id;

    char input[MESSAGE_BUFFER];
    char message[MESSAGE_BUFFER];
    int response;

    printf("New player session started\n");

    bool correct_username = false;
    do{
        //Get the username
        response = recv(p_data->socket_fd, input, MESSAGE_BUFFER, 0);
        if (response <= 0) {
            printf("Unnamed player disconnected or recv() failed.\n");
            pthread_exit(NULL);
        }

        correct_username = (find_player(input)==NULL);
        if(!correct_username){
            strcpy(message,"Sorry, that username is taken: try a different one");
            send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
        }
    
    }while(!correct_username);

    strcpy(p_data->username,input);
    printf("Got the username for %s\n", p_data->username);

    
    char *opponent_name;
    while(true){

        while(p_data->in_match); //We now handle the input from the game thread. 

        response = recv(p_data->socket_fd, input, MESSAGE_BUFFER, 0);
        
        //Error/disconnection handling
        if (response <= 0) {
            printf("%s disconnected or recv() failed.\n",p_data->username);
            break;
        }

        //Comands handling
        if (strncmp(input, "/quit", 5) == 0) {
            printf("Closing connection with %s\n",p_data->username);
            break;
        }
        if (strncmp(input, "/playagainst ",13) == 0) {
            opponent_name = after_space(input);
            
            printf("Selected opponent: %s\n", opponent_name);
            player_data *opponent = find_player(opponent_name);
            
            if(opponent!=NULL){
                printf("Opponent found.\n");

                if(!opponent->in_match){

                    strcpy(message, "Player ");
                    strcat(message, p_data->username);
                    strcat(message, " wants to play. Type /accept to accept or anything else to reject.");

                    send(opponent->socket_fd, message, MESSAGE_BUFFER, 0);
                    
                    response = recv(opponent->socket_fd, input, MESSAGE_BUFFER, 0);
                    if (response <= 0) {
                        printf("%s disconnected or recv() failed.\n",opponent->username);
                        strcpy(message, "Your opponent timed out.");
                        send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                        continue;
                    }

                    if (strncmp(input, "/accept",7) == 0) {
                        game_session_data *g_data = malloc(sizeof(game_session_data));
                        g_data->player1 = p_data;
                        g_data->player2 = opponent;

                        pthread_t game;
                        int create_result = pthread_create(&game, NULL, game_session, (void *)g_data);
                        
                        if (create_result){
                            printf("Error while trying to create game session %d\n", create_result);
                            
                            free(g_data);
                            g_data = NULL;
                            
                            strcpy(message, "Sorry, couldn't start game.");
                            send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                            send(opponent->socket_fd, message, MESSAGE_BUFFER, 0);
                        }else{

                            printf("Game started successfully.\n");
                            p_data->in_match = true;
                            opponent->in_match = true;

                        }

                    }else{
                        strcpy(message, "Opponent didn't accept the invitation.");
                        send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                    }

                }else{
                    strcpy(message, "Sorry, your opponent is currently in a match.");
                    send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                }

            }else{
                strcpy(message, "Could not find opponent.");
                send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
            }

        }
        printf("%s: %s\n",p_data->username, input);
    }

    printf("Shutting player %s session down.\n",p_data->username);
    
    connectedPlayers--;
    free(p_data);
    players[index]=NULL; //Apparently good practice
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

    //Seting a timeout
    /*struct timeval tv;
    tv.tv_sec = 15;
    tv.tv_usec = 0;
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);*/

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
                printf("Error while trying to create player session %d\n", create_result);
                free(players[index]);
                players[index]=NULL; //Apparently good practice
                continue;
            }
            
            connectedPlayers++;

        }

    }

    // Close socket, kill thread and free memory
    close(socket_fd);
    for(int i = 0; i < PLAYERS_MAX_NUMBER; i++){
        free(players[i]);
        players[i]=NULL; //Apparently good practice
    }
    pthread_exit(NULL);
    return 0;
}