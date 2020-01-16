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
#define USERNAME_SIZE 16
#define CLIENT_ADDRESS_LENGTH 100
#define PLAYERS_MAX_NUMBER 32

//Player data struct
typedef struct player_data{
    pthread_t *thread;
    int id;
    int socket_fd;
    bool in_match;
    char username[USERNAME_SIZE];
}player_data;

//Game session data
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

//PLayer session declaration
void * player_session(void *pl_data);

//Game session thread
void * game_session(void *ga_data){
    pthread_detach(pthread_self());

    game_session_data *g_data = (game_session_data*)ga_data;

    player_data *p1 = g_data->player1;
    player_data *p2 = g_data->player2;

    char input[MESSAGE_BUFFER];
    char message[MESSAGE_BUFFER];

    int response,n;

    strcpy(message, "You are now in a match against ");
    strcat(message, p2->username);
    response = send(p1->socket_fd, message, MESSAGE_BUFFER, 0);
    if(response == -1){
        printf("The server has experienced an unexpected crash.\n"); 
        pthread_exit(NULL);
    }     

    strcpy(message, "You are now in a match against ");
    strcat(message, p1->username);
    response = send(p2->socket_fd, message, MESSAGE_BUFFER, 0);
    if(response == -1){
        printf("The server has experienced an unexpected crash.\n"); 
        pthread_exit(NULL);
    }     

    //Stop the players' sessions
    pthread_cancel(*p1->thread);
    pthread_cancel(*p2->thread);

    //Initial board arrangement, 2 - white 4 - black,
    int board[8][8] = 
    {
          {0 , 4 , 0 , 4 , 0 , 4 , 0 , 4},
          {4 , 0 , 4 , 0 , 4 , 0 , 4 , 0},
          {0 , 4 , 0 , 4 , 0 , 4 , 0 , 4},
          {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0},
          {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0},
          {2 , 0 , 2 , 0 , 2 , 0 , 2 , 0},
          {0 , 2 , 0 , 2 , 0 , 2 , 0 , 2},
          {2 , 0 , 2 , 0 , 2 , 0 , 2 , 0}
    };

    int current_color;
    bool white = rand() & 1; //So that a random player starts.
    player_data *current_player;
    player_data *current_opponent;
    while(true){
        
        current_player = (white)?p1:p2;
        current_opponent = (white)?p2:p1;
        int current_color = (white)?2:4;

        strcpy(message, "Your turn.");
        response = send(current_player->socket_fd, message, MESSAGE_BUFFER, 0);
        if(response == -1){
            printf("The server has experienced an unexpected crash.\n"); 
            pthread_exit(NULL);
        }     

        strcpy(message, "Opponent's turn.");
        response = send(current_opponent->socket_fd, message, MESSAGE_BUFFER, 0);  
        if(response == -1){
            printf("The server has experienced an unexpected crash.\n"); 
            pthread_exit(NULL);
        }     

        n = 0;
        while(n!=MESSAGE_BUFFER){
            response = read(current_player->socket_fd, &input[n], MESSAGE_BUFFER-n);
            if(response==-1){
                printf("Game session end due to recv() error.\n"); 
                break;
            }
            n+=response;
        }

        //Quitting handling
        if(strncmp(input, "/end", 4) == 0) {
            printf("Ending game.\n");
            break;
        }

        //Finding if the player has any available captures that he has to make
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 8; j++){
                if(board[i][j]==current_color){

                }
            }
        }

        //Move handling
        if(strncmp(input, "/move", 5) == 0) {
            
        }

        white=!white;

    }

    g_data->player1->in_match = false;
    g_data->player2->in_match = false;

    //Restart player sessions
    pthread_create(p1->thread, NULL, player_session, (void *)p1);
    pthread_create(p2->thread, NULL, player_session, (void *)p2);

    free(g_data);
    pthread_exit(NULL); 
} 

void * player_session(void *pl_data){
    pthread_detach(pthread_self());

    player_data *p_data = (player_data*)pl_data;
    int index = p_data->id;

    char input[MESSAGE_BUFFER];
    char message[MESSAGE_BUFFER];
    int response,n;

    if(strcmp(p_data->username,"")==0){ //So that if we restart the session we don't ask for the username again

        bool correct_username = false;
        do{
            //Get the username
            /*response = recv(p_data->socket_fd, input, MESSAGE_BUFFER, 0);
            if (response <= 0) {
                printf("Unnamed player disconnected or recv() failed.\n");
                pthread_exit(NULL);
            }*/
            n = 0;
            while(n!=MESSAGE_BUFFER){
                response = read(p_data->socket_fd, &input[n], MESSAGE_BUFFER-n);
                if(response==-1){
                    printf("Unnamed player disconnected or recv() failed.\n");
                    pthread_exit(NULL);
                }
                n+=response;
                //printf("%d %d %c\n",MESSAGE_BUFFER,n,input[strlen(input)-1]);
            }

            correct_username = (find_player(input)==NULL);
            if(!correct_username){
                strcpy(message,"Sorry, that username is taken: try a different one");
                response = send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                if(response == -1){
                    printf("The server has experienced an unexpected crash.\n"); 
                    pthread_exit(NULL);
                } 
            }
        
        }while(!correct_username);

        strcpy(p_data->username,input);
        printf("Got the username for %s\n", p_data->username);
        printf("Player session started\n");

    }else{
        printf("Player session restarted.\n");
    }

    
    char *opponent_name;
    while(true){

        /*response = recv(p_data->socket_fd, input, MESSAGE_BUFFER, 0);
        if (response <= 0) {
            printf("%s disconnected or recv() failed.\n",p_data->username);
            break;
        }*/
        n = 0;
        while(n!=MESSAGE_BUFFER){
            response = read(p_data->socket_fd, &input[n], MESSAGE_BUFFER-n);
            if(response==-1){
                printf("%s disconnected or recv() failed.\n",p_data->username);
                break;
            }
            n+=response;
        }

        //Commands handling
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

                    response = send(opponent->socket_fd, message, MESSAGE_BUFFER, 0);
                    if(response == -1){
                        printf("The server has experienced an unexpected crash.\n"); 
                        pthread_exit(NULL);
                    } 
                    
                    n = 0;
                    while(n!=MESSAGE_BUFFER){
                        response = read(opponent->socket_fd, &input[n], MESSAGE_BUFFER-n);
                        if(response==-1){
                            printf("%s disconnected or recv() failed.\n",opponent->username);
                            strcpy(message, "Your opponent timed out.");
                            response = send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                            if(response == -1){
                                printf("The server has experienced an unexpected crash.\n"); 
                                pthread_exit(NULL);
                            } 
                            continue;
                        }
                        n+=response;
                    }
                    /*response = recvfrom(opponent->socket_fd, input, MESSAGE_BUFFER, 0, NULL, NULL);
                    if (response <= 0) {
                        printf("%s disconnected or recv() failed.\n",opponent->username);
                        strcpy(message, "Your opponent timed out.");
                        response = send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                        if(response == -1){
                            printf("The server has experienced an unexpected crash.\n"); 
                            pthread_exit(NULL);
                        } 
                        continue;
                    }*/

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
                            response = send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                            if(response == -1){
                                printf("The server has experienced an unexpected crash.\n"); 
                                pthread_exit(NULL);
                            } 
                            response = send(opponent->socket_fd, message, MESSAGE_BUFFER, 0);
                            if(response == -1){
                                printf("The server has experienced an unexpected crash.\n"); 
                                pthread_exit(NULL);
                            } 
                        }else{

                            printf("Game started successfully.\n");
                            p_data->in_match = true;
                            opponent->in_match = true;

                        }

                    }else{
                        strcpy(message, "Opponent didn't accept the invitation.");
                        response = send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                        if(response == -1){
                            printf("The server has experienced an unexpected crash.\n"); 
                            pthread_exit(NULL);
                        } 
                    }

                }else{
                    strcpy(message, "Sorry, your opponent is currently in a match.");
                    response = send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                    if(response == -1){
                        printf("The server has experienced an unexpected crash.\n"); 
                        pthread_exit(NULL);
                    } 
                }

            }else{
                strcpy(message, "Could not find opponent.");
                response = send(p_data->socket_fd, message, MESSAGE_BUFFER, 0);
                if(response == -1){
                    printf("The server has experienced an unexpected crash.\n"); 
                    pthread_exit(NULL);
                } 
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
            players[index]->thread = malloc(sizeof(pthread_t));

            int create_result = pthread_create(players[index]->thread, NULL, player_session, (void *)players[index]);

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