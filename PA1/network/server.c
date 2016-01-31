#include<stdio.h>
#include<string.h>    
#include<stdlib.h>    
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
 
#include<pthread.h> //for threading , link with lpthread
 
void *connection_handler(void *);
typedef struct thread_data {
    int sock_fd, block_size;
} thread_data;

int main(int argc , char *argv[])
{
    int socket_fd , new_socket , c , *thread_data, port;
    struct sockaddr_in server , client;
    
    if (argc < 2){
        printf("%s","port required\n");
        exit(1);
    }
    port = atoi(argv[1]);
    
    //Create socket
    socket_fd = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_fd == -1){
        printf("Unable to create the socket");
        exit(1);
    }
     
    //Create sockaddr info
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( port );//default port
    printf("server listening port %i, address %i\n", server.sin_port, server.sin_addr.s_addr);
    
    //Bind socket
    if( bind(socket_fd,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("binding failed\n");
        exit(1);
    }
    printf("successful binding\n");
     
    listen(socket_fd , 5);
    printf("Listening...\n");
    
    //Accept and incoming connection
    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_fd, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        printf("Connection accepted\n");
         
        pthread_t handler_thread;
         
        thread_data = malloc(1);
        *thread_data = new_socket;
        
        if(pthread_create( &handler_thread , NULL ,  connection_handler , (void*) thread_data) < 0){
            printf("error creating thread\n");
            exit(1);
        }
         
        printf("Thread created\n");
    }
     
    if (new_socket<0)
    {
        printf("accept failed");
        exit(1);
    }
     
    return (EXIT_SUCCESS);
}
 
//receive all data from client
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int block_size, read_count,i; 
    char* buffer;
    
    printf("Start receiving from client\n");
    read(sock,&block_size,sizeof(int));
    printf("Block size: %i\n",block_size);
    read(sock,&read_count,sizeof(int));
    printf("Block count: %i\n",read_count);
    buffer = malloc(sizeof(char)*block_size);
    for(i=0;i<read_count;i++){
        int n = read(sock,buffer,block_size);
        if(n<0){
            printf("error reading\n");
            exit(1);
        }
    }
    printf("End receiving from client\n"); 
    
    //close(sock);
    free(socket_desc);
    free(buffer);
    pthread_exit(NULL);
}
