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
#define USERNAME_SIZE 10

typedef struct thread_data{
  int socket_fd;
}thread_data;

void sending(void * data){
  pthread_detach(pthread_self());

  char message[MESSAGE_BUFFER];
  thread_data *t_data = (thread_data*)data;

  //printf("Sending started.\n");
  int response, n;

  while (fgets(message, MESSAGE_BUFFER, stdin) != NULL) {
    message[strlen(message) - 1] = 0; // Remove newline char from end of string

    n = 0;
    while(n!=MESSAGE_BUFFER){
      response = write(t_data->socket_fd, &message[n], MESSAGE_BUFFER-n);
      if(response==-1){
        printf("Sending error.\n");
        break;
      }
      n+=response;
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

  //printf("Receiving started.\n");
  int response,n;

  while(true){

    n = 0;
    while(n!=MESSAGE_BUFFER){
      response = read(t_data->socket_fd, &message[n], MESSAGE_BUFFER-n);
      if(response<=0){
        printf("Disconnected from server.\n");
        break;
      }
      n+=response;
    }

    printf("%s\n", message);
  }

  free(data);
  pthread_exit(NULL); 
}

int main(int argc, char**argv) {
    
  int connection_socket_descriptor;
  struct sockaddr_in server_address;
  struct hostent* server_host_entity;
  char username[USERNAME_SIZE];

  int response, n;

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
  tv.tv_usec = 0;*/

  //char reuse_addr_val = 1;
  //setsockopt(connection_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

  //Connect to server
  response = connect(connection_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
  if(response < 0){
    fprintf(stderr, "connect() failed: %s\n", strerror(errno));
    exit(1);
  }else{
    printf("Connected\n");
  }

  // Get username
  printf("Enter your user name: ");
  fgets(username, USERNAME_SIZE, stdin);
  username[strlen(username) - 1] = 0; // Remove newline char from end of string

  //Send the username 
  n = 0;
  while(n!=MESSAGE_BUFFER){
    response = write(connection_socket_descriptor, &username[n], MESSAGE_BUFFER-n);
    if(response==-1){
      printf("Disconnected from server.\n");
      //This will be changed once we get a re-connect option
      pthread_exit(NULL);
      return 0;
    }
    n+=response;
  }
  
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