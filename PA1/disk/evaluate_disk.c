/* 
 * File:   main.c
 * Author: mrosenfeld
 *
 * Created on January 26, 2016, 9:24 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#define FILE_SIZE 30000000//in bytes
static const char filename[] = "dummy_file";

typedef enum {
    SEQUENTIAL, RANDOM
} strategy;
const char* strategyNames[] = {"Sequential", "Random"};

typedef enum {
    READ, WRITE
} op_type;
const char* op_type_names[] = {"Read", "Write"};

void measure(int block_size, const int seconds_per_iteration, const int threads, const strategy strategy, const op_type op_type, double* throughput, double* latency);
void *perform(void *arg);//thread

int flag = 1;

//structure to pass all data to the thread, to allow the thread perform the memcpy operations in different sections of the memory
typedef struct thread_data {
    int from, to, block_size;
    int operation_count;
    strategy strategy;
    op_type op_type;
    char* file_name;
    FILE* file;
} thread_data;

int main(int argc, char** argv) {

    FILE *fp;
    double *throughput, *latency;
    int block_sizes[3] = {1, 1024, 1048576};
    int threads[2] = {1, 2}; 
    int i, j, k, m, iterations, seconds_per_iteration, it;

    if(argc < 3) {
            printf("Usage: [no of iterations] [seconds per iteration]\n");
            exit(1);
    }

    iterations = atoi(argv[1]);
    seconds_per_iteration = atoi(argv[2]);
	
    throughput = (double*) malloc(sizeof (double));
    latency = (double*) malloc(sizeof (double));
    fp = fopen("result.csv", "w+"); //write the results in csv file

    //iterate through all possible combinations (total of 12 combinations)
    for (i = 0; i < (sizeof (block_sizes) / sizeof (int)); i++) {
        for (j = 0; j < (sizeof (threads) / sizeof (int)); j++) {
            for (m = READ; m <= WRITE; m++) {
                for (k = SEQUENTIAL; k <= RANDOM; k++) {
                    double total_th = 0.0;
                    for(it=0;it<iterations;it++){
                        measure(block_sizes[i], seconds_per_iteration, threads[j], k, m, throughput, latency);
                        total_th += *throughput;
                    }
                    fprintf(fp, "%i, %i, %s, %f, %f\n", block_sizes[i], threads[j], strategyNames[k], total_th/(double)iterations, *latency);
                }
            }
        }
    }
    free(throughput);
    free(latency);
    fclose(fp);
    return (EXIT_SUCCESS);
}

void *perform(void *arg) {
    thread_data *tdata = (thread_data *) arg;
    int block_number, block_count, block_size;
    int i;
    int file;
    char* buffer;
    
    block_size = tdata->block_size;
    
    file=open(filename, tdata->op_type == READ ? O_RDONLY:O_RDWR);
    buffer = malloc(block_size *  sizeof(char));
            
    block_count = tdata->to - tdata->from + 1;
    srand(time(NULL));
    for (i = 0; flag; i++) {
        if (tdata->strategy == SEQUENTIAL){
            if((int)i%block_count == 0) 
                lseek(file,tdata->from * block_size,SEEK_SET);
        }
        else{
            block_number = ((rand() % block_count) + tdata->from) ;
            lseek(file,block_number * block_size,SEEK_SET);
        }
        if(tdata->op_type == READ){
            if(read(file, buffer, block_size) < 0)       
            {
                printf("error1");
		exit(1);
            }
        }
        else
        {
            if(write(file, buffer, block_size) < 0)
            {
                printf("error2");
                exit(1);
            }
        }
    }
    tdata->operation_count = i;
    free(buffer);
    close(file);
    pthread_exit(NULL);
}

void measure(const int block_size, const int seconds, const int thread_count, const strategy strategy, const op_type op_type, double* throughput, double* latency) {
    
    FILE *file; //file to perform the operations
    struct timeval start_time, end_time;//times
    int block_count, n;
    int totalOperations;
    double total_seconds;
    pthread_t *threads = (pthread_t*) malloc(thread_count * sizeof (pthread_t));
    thread_data *data = (thread_data*) malloc(thread_count * sizeof (thread_data));

    block_count = FILE_SIZE / block_size;
	
    //create dummy file
    file=fopen(filename, "w");
    fseek(file, FILE_SIZE-1, SEEK_SET);
    fputc('\n', file);
    fclose(file);
	
    gettimeofday(&start_time, NULL);
	//create threads
    for (n = 0; n < thread_count; n++) {  
        data[n].strategy = strategy;
        data[n].op_type = op_type;
        data[n].block_size = block_size;
        data[n].from = n * (block_count / thread_count);
        data[n].to = (n + 1)*(block_count / thread_count) - 1;
        pthread_create(&threads[n], NULL, perform, (void *) &data[n]);
    }

    flag=1;
    sleep(seconds);
    flag = 0;
	
    //wait threads and count operations
    totalOperations = 0;
    for (n = 0; n < thread_count; n++) {
        pthread_join(threads[n], NULL);
	totalOperations += data[n].operation_count;
    }

    gettimeofday(&end_time, NULL);

    total_seconds =
         (double) (end_time.tv_usec - start_time.tv_usec) / 1000000 +
         (double) (end_time.tv_sec - start_time.tv_sec);
    //print in console
    printf("\nBlock size: %d, threads: %d, operation: %s, strategy: %s\n", block_size, thread_count, op_type_names[op_type],strategyNames[strategy]);
    printf("Total seconds: %f\n", total_seconds);
    printf("Mb: %f\n", (totalOperations * (double) block_size / (double) 1048576));
    printf("Throughput: %f MB/sec\n", (totalOperations * (double) block_size / 1048576) / (double) total_seconds);
    printf("Latency: %f ms\n", (total_seconds / (double) totalOperations) * 1000);
    *throughput = (totalOperations * (double) block_size / 1048576) / (double) total_seconds;
    *latency = ( total_seconds / (double) totalOperations) * 1000;
    
    free(data);
    free(threads);
}
