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

player_data* connectedPlayers[2];

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

    connectedPlayers[0] = p_data;

    // = strdup("test");

    player_session(p_data);
    //p_data->username = 

}

int main (int argc, char *argv[])
{
   int connection_socket_descriptor;
   int connect_result;
   struct sockaddr_in server_address;
   struct hostent* server_host_entity;

   server_host_entity = gethostbyname(argv[1]);

   connection_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);

   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = AF_INET;
   memcpy(&server_address.sin_addr.s_addr, server_host_entity->h_addr, server_host_entity->h_length);
   server_address.sin_port = htons(atoi(argv[2]));

   connect_result = connect(connection_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));


   close(connection_socket_descriptor);
   return 0;

}