#include<stdio.h>
#include<string.h>
#include<stdlib.h>  
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>  
#include <pthread.h>
#include <errno.h>
typedef enum {
    TCP, UDP
} protocol;

//structure to pass all data to the thread
typedef struct thread_data {
    int from, to, operation_count, block_size, server_port_tcp, server_port_udp;
    protocol protocol;
    char* server_address;
} thread_data;

const char* protocolNames[] = {"TCP", "UDP"};
void measure(int block_size, const double operations, const int threads, const protocol protocol, char* server_address, const int server_port_tcp, const int server_port_udp, double* throughput, double* latency);
void *perform(void *arg);//thread

int main(int argc, char** argv) {

    FILE *fp;
    double *throughput, *latency;
    int block_sizes[3] = {1,1024, 65507};
    int threads[2] = {1,2};
    double operations[3] = {1000000,1000000,1000000}; //will perform different amount of operations for different block sizes
    int i, j, k, it, iterations, server_port_tcp, server_port_udp;
    char* server_address;

     if (argc < 5){
        printf("%s","Usage: client [iterations] [server address] [port_tcp] [port_udp] \n");
        exit(1);
    }
    
    server_address = malloc(sizeof(char)*32);
    server_address = argv[2];
    server_port_tcp = atoi(argv[3]);
    server_port_udp = atoi(argv[4]);
    
    iterations = atoi(argv[1]);
    
    throughput = (double*) malloc(sizeof (double));
    latency = (double*) malloc(sizeof (double));
    fp = fopen("result.csv", "w+"); //write the results in csv file

    //iterate through all possible combinations (total of 12 combinations)
    for (i = 0; i < (sizeof (block_sizes) / sizeof (int)); i++) {
        for (j = 0; j < (sizeof (threads) / sizeof (int)); j++) {
            for (k = TCP; k <= UDP; k++) {
                double total_th = 0.0;
                for(it=0;it<iterations;it++){
                    measure(block_sizes[i], operations[i], threads[j], k, server_address, server_port_tcp, server_port_udp, throughput, latency);
                    total_th += *throughput;
		}
                
                fprintf(fp, "%i, %i, %s, %f, %f\n", block_sizes[i], threads[j], protocolNames[k], total_th/(double)iterations, *latency);

            }
        }
    }
    free(throughput);
    free(latency);
    fclose(fp);
    return (EXIT_SUCCESS);
}

void measure(int block_size, const double operations, const int thread_count, const protocol protocol, char* server_address, const int server_port_tcp, const int server_port_udp, double* throughput, double* latency){

    int n;
    clock_t start_time, end_time;//times
    double total_seconds;
    pthread_t *threads = (pthread_t*) malloc(thread_count * sizeof (pthread_t));
    thread_data *data = (thread_data*) malloc(thread_count * sizeof (thread_data));
    
    start_time = clock();
    printf("start %zu\n",start_time);
    //create threads
    for (n = 0; n < thread_count; n++) {
        data[n].operation_count = operations / thread_count;
        data[n].protocol = protocol;
        data[n].block_size = block_size;
        data[n].server_address = server_address;
        data[n].server_port_tcp = server_port_tcp;
        data[n].server_port_udp = server_port_udp;
        pthread_create(&threads[n], NULL, perform, (void *) &data[n]);
    }
    
    //wait threads
    for (n = 0; n < thread_count; n++) {
        pthread_join(threads[n], NULL);
    }

    end_time = clock();
    printf("end %zu\n",end_time);

    total_seconds = (end_time - start_time) / ((double) CLOCKS_PER_SEC);
    //print in console
    printf("\nBlock size: %d, threads: %d, protocol: %s\n", block_size, thread_count, protocolNames[protocol]);
    printf("Total seconds: %f\n", total_seconds);
    printf("Mb: %f\n", (operations * (double) block_size / (double) 1048576));
    printf("Throughput: %f\n", (operations * (double) block_size / 1048576) / (double) total_seconds);
    *throughput = (operations * (double) block_size / 1048576) / (double) total_seconds;
    free(threads);
    free(data);
}

void *perform(void *arg) {
    int socket_desc;
    struct sockaddr_in server;  
    protocol prot;
    int server_port_tcp, server_port_udp, block_count, block_size,i;
    char *server_address, buffer[65536];
    thread_data *tdata = (thread_data *) arg;
    struct sockaddr_in dest;
    
    server_address = tdata->server_address;
    server_port_tcp = tdata->server_port_tcp;
    server_port_udp = tdata->server_port_udp;
    block_size = tdata->block_size;
    block_count = tdata->operation_count;
    prot = tdata->protocol; 
    
    
    //Create socket
    socket_desc = socket(AF_INET , prot == TCP? SOCK_STREAM : SOCK_DGRAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    
    server.sin_addr.s_addr = inet_addr(server_address);
    server.sin_family = AF_INET;
    server.sin_port = htons( prot == TCP? server_port_tcp : server_port_udp );
 
    //Connect to remote server if tcp
    if(prot == TCP){
        if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
        {
            printf("connect error\n");
            exit(1);
        }
    }
     
    int option = 1;
    if(setsockopt(socket_desc, IPPROTO_IP, SO_SNDBUF, (char*)&option, sizeof(int)) < 0) {
        printf("Can't set socket options:%d:%s\n", errno, strerror(errno));
        exit(1);
    }
    
    //Send block_size
    send(socket_desc , &block_size , sizeof(int) , 0);
    send(socket_desc , &block_count , sizeof(int) , 0);
    //buffer = (char*)malloc(block_size*block_count*sizeof(char));
    for(i=0;i<block_count;i++){
        int n;
        if(prot == TCP) n = write(socket_desc , buffer  , block_size);
        else n = sendto(socket_desc,buffer, block_size,0,(struct sockaddr*)&server, sizeof (dest));
        //printf("%i,",n);
        if(n<0){
            printf("Error writing: %i\n",errno);
            exit(1);
        }
        if(n<block_size){
            printf("Unable to write all block %i %i",n,block_size);
            exit(1);
        }
        
    }
   // free(buffer);
    close(socket_desc);
    pthread_exit(NULL);
}



