#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"

#define USERNAME_BUFFER 10

//Player data struct
typedef struct player_data{
    int id;
    int socket_fd;
    char username[USERNAME_BUFFER];
    void* players;
}player_data;

void player_session(void *connectedPlayers){
    player_data *players = (player_data *)connectedPlayers;
    printf("%s\n",&players[0].username);
}

int main(int argc, char**argv) {

    //pointer
    player_data *p_data = malloc(sizeof(player_data));
    player_data* connectedPlayers[2];
    
    p_data->socket_fd = 1;
    p_data->id = 2;
    strcpy(p_data->username,"test");
    
   

    connectedPlayers[0] = p_data;

     p_data->players = (void*)connectedPlayers;

    printf("%s\n", connectedPlayers[0]->username);

    // = strdup("test");

    player_session(connectedPlayers);
    //p_data->username = 

}
