#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#define MESSAGE_BUFFER 500
#define USERNAME_BUFFER 10

//If this is true we shut down the client (i know i know, global variables, eww)
bool shutDown = false;

typedef struct {
    char* prompt;
    int socket;
} thread_data;

void game_session(){

  if(){
    while(){

    }
  }else{

  }

}

// Get message from stdin and send to server
void * send_message(char prompt[USERNAME_BUFFER+4], int socket_fd, struct sockaddr_in *address) {
  printf("%s", prompt);
  char message[MESSAGE_BUFFER];
  char final_message[MESSAGE_BUFFER+USERNAME_BUFFER+1];

  //Send the desired opponent's name
  send(socket_fd, , strlen(final_message)+1, 0);

  while (fgets(message, MESSAGE_BUFFER, stdin) != NULL) {
      memset(final_message,0,strlen(final_message)); // Clear final message buffer
      strcat(final_message, prompt);
      strcat(final_message, message);
      printf("\n%s", prompt);
      if (strncmp(message, "/quit", 5) == 0) {
        printf("Closing connection...\n");
        exit(0);
      }
      send(socket_fd, final_message, strlen(final_message)+1, 0);
  }
}

void * receive(void * threadData) {
    int socket_fd, response;
    char message[MESSAGE_BUFFER];
    thread_data* pData = (thread_data*)threadData;
    socket_fd = pData->socket;
    char* prompt = pData->prompt;
    memset(message, 0, MESSAGE_BUFFER); // Clear message buffer

    // Print received message
    while(1) {
        response = recvfrom(socket_fd, message, MESSAGE_BUFFER, 0, NULL, NULL);
        if (response == -1) {
          fprintf(stderr, "recv() failed: %s\n", strerror(errno));
          break;
        } else if (response == 0) {
            printf("\nPeer disconnected\n");
            break;
        } else {
            printf("\nServer> %s", message);
            printf("%s", prompt);
            fflush(stdout); // Make sure "User>" gets printed
        }
    }
}

int main(int argc, char**argv) {
    long port = strtol(argv[2], NULL, 10);
    struct sockaddr_in address, cl_addr;
    char * server_address;
    int socket_fd, response;
    char prompt[USERNAME_BUFFER+4];
    char username[USERNAME_BUFFER];
    char opponent[USERNAME_BUFFER];
    pthread_t thread;

    // Check for required arguments
    if (argc < 3) {
      printf("Usage: client ip_address port_number\n");
      exit(1);
    }

    // Get user handle
    printf("Enter your user name: ");
    fgets(username, USERNAME_BUFFER, stdin);
    username[strlen(username) - 1] = 0; // Remove newline char from end of string
    strcpy(prompt, username);
    strcat(prompt, "> ");

    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address = argv[1];
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_address);
    address.sin_port = htons(port);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    //Connect to server
    int response = connect(socket_fd, (struct sockaddr *) address, sizeof *address);
    if(response < 0){
      fprintf(stderr, "connect() failed: %s\n", strerror(errno));
      exit(1);
    }else{
      printf("Connected\n");
    }

    while(!shutDown){
      //Get opponent handle
      printf("Enter your desired opponent's user name: ");
      fgets(opponent, USERNAME_BUFFER, stdin);
      opponent[strlen(opponent) - 1] = 0; // Remove newline char from end of string

      game_session();

    }

    // Create data struct for new thread
    //thread_data data;
    //data.prompt = prompt;
    //data.socket = socket_fd;

    // Create new thread to receive messages
    //pthread_create(&thread, NULL, receive, (void *) &data);

    // Send message
    //send_message(prompt, socket_fd, &address);

    // Close socket and kill thread
    close(socket_fd);
    pthread_exit(NULL);
    return 0;

}