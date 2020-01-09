#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "stdbool.h"
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

typedef struct thread_data{
  int socket_fd;
}thread_data;

void sending(void * data){
  pthread_detach(pthread_self());

  char message[MESSAGE_BUFFER];
  thread_data *t_data = (thread_data*)data;

  printf("Sending started.\n");
  int response;

  while (fgets(message, MESSAGE_BUFFER, stdin) != NULL) {
    message[strlen(message) - 1] = 0; // Remove newline char from end of string
    response = send(t_data->socket_fd, message, MESSAGE_BUFFER, 0);
    if(response == -1){
      printf("Sending error.\n");
      break;
    }
    if (strncmp(message, "/quit", 5) == 0) {
      printf("Closing client.\n");
      break;
    }
  }

  free(data);
  pthread_exit(NULL); 
}

void * receiving(void * data){
  pthread_detach(pthread_self());  
  
  char message[MESSAGE_BUFFER];
  thread_data *t_data = (thread_data*)data;

  printf("Receiving started.\n");
  int response;

  while(true){
    response = recvfrom(t_data->socket_fd, message, MESSAGE_BUFFER, 0, NULL, NULL);
    if(response <= 0){
      printf("Disconnected from server.\n");
      break;
    }
    printf("%s\n", message);
  }

  free(data);
  pthread_exit(NULL); 
}

// Get message from stdin and send to server
/*void * send_message(char prompt[USERNAME_BUFFER+4], int socket_fd, struct sockaddr_in *address) {
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
}*/

int main(int argc, char**argv) {
    
  int connection_socket_descriptor;
  struct sockaddr_in server_address;
  struct hostent* server_host_entity;
  char username[USERNAME_BUFFER];

  // Check for required arguments
  if (argc < 3) {
    printf("Usage: client ip_address port_number\n");
    exit(1);
  }

  server_host_entity = gethostbyname(argv[1]);
  connection_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);

  memset(&server_address, 0, sizeof(struct sockaddr));
  server_address.sin_family = AF_INET;
  memcpy(&server_address.sin_addr.s_addr, server_host_entity->h_addr, server_host_entity->h_length);
  server_address.sin_port = htons(atoi(argv[2]));

  //Seting a timeout
  /*struct timeval tv;
  tv.tv_sec = 15;
  tv.tv_usec = 0;
  setsockopt(connection_socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);*/

  //Connect to server
  int response = connect(connection_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
  if(response < 0){
    fprintf(stderr, "connect() failed: %s\n", strerror(errno));
    exit(1);
  }else{
    printf("Connected\n");
  }

  // Get username
  printf("Enter your user name: ");
  fgets(username, USERNAME_BUFFER, stdin);
  username[strlen(username) - 1] = 0; // Remove newline char from end of string

  //Send the username 
  send(connection_socket_descriptor, username, USERNAME_BUFFER, 0);
  
  //Start the client player session
  thread_data *t_data = malloc(sizeof(t_data));
  t_data->socket_fd = connection_socket_descriptor;

  pthread_t rec_thread;
  pthread_create(&rec_thread, NULL, receiving, (void *)t_data);

  sending((void *)t_data);

  //Closing the client
  printf("Closing client.\n");
  
  close(connection_socket_descriptor);
  pthread_exit(NULL);
  return 0;

}