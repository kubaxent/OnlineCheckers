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
}player_data;

//Game session data
typedef struct game_session_data{
    player_data *player1;
    player_data *player2;
}game_session_data;

//Game session thread
void  game_session(void *ga_data){

    game_session_data *g_data = (game_session_data*)ga_data;

    printf("%s\n", g_data->player1->username);
    printf("%d\n", g_data->player1->id);
}

void player_session(void *pl_data){

    player_data *p_data = (player_data*)pl_data;

    game_session_data *g_data = malloc(sizeof(game_session_data));
    g_data->player1 = p_data;
    g_data->player2 = p_data;

    game_session(g_data);
}

int main(int argc, char**argv) {

    //pointer
    player_data *p_data = malloc(sizeof(player_data));
    
    p_data->socket_fd = 1;
    p_data->id = 2;
    strcpy(p_data->username,"test");
    // = strdup("test");

    player_session(p_data);
    //p_data->username = 

}