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

void measure(int block_size, const double operations, const int threads, const strategy strategy, const op_type op_type, double* throughput, double* latency);
void *perform(void *arg);//thread

//structure to pass all data to the thread, to allow the thread perform the memcpy operations in different sections of the memory
typedef struct thread_data {
    int from, to, operation_count, block_size;
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
    double operations[3] = {FILE_SIZE, FILE_SIZE/1024, 1200}; //will perform different amount of operations for different block sizes
    int i, j, k, m;

    throughput = (double*) malloc(sizeof (double));
    latency = (double*) malloc(sizeof (double));
    fp = fopen("result.csv", "w+"); //write the results in csv file

    //iterate through all possible combinations (total of 12 combinations)
    for (i = 0; i < (sizeof (block_sizes) / sizeof (int)); i++) {
        for (j = 0; j < (sizeof (threads) / sizeof (int)); j++) {
            for (m = READ; m <= WRITE; m++) {
                for (k = SEQUENTIAL; k <= RANDOM; k++) {
                    measure(block_sizes[i], operations[i], threads[j], k, m, throughput, latency);
                    fprintf(fp, "%i, %i, %s, %f, %f\n", block_sizes[i], threads[j], strategyNames[k], *throughput, *latency);
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
    int i, block_number, block_count, block_size;
    FILE *file;
    char* buffer;
    
    block_size = tdata->block_size;
    
    file=fopen(filename, tdata->op_type == READ ? "r":"r+");
    buffer = malloc(block_size *  sizeof(char));
            
    block_count = tdata->to - tdata->from + 1;
    srand(time(NULL));
    for (i = 0; i < tdata->operation_count; i++) {
        if (tdata->strategy == SEQUENTIAL){
            if(i%block_count == 0) 
                fseek(file,tdata->from * block_size,SEEK_SET);
        }
        else{
            block_number = ((rand() % block_count) + tdata->from) ;
            fseek(file,block_number * block_size,SEEK_SET);
        }
        if(tdata->op_type == READ)
            fread(buffer, block_size,1,file);
        else
            fwrite(buffer, block_size,1,file);
    }
    free(buffer);
    fclose(file);
    pthread_exit(NULL);
}

void measure(const int block_size, const double operations, const int thread_count, const strategy strategy, const op_type op_type, double* throughput, double* latency) {
    
    FILE *file; //file to perform the operations
    clock_t start_time, end_time;//times
    int block_count, n;
    double total_seconds;
    pthread_t *threads = (pthread_t*) malloc(thread_count * sizeof (pthread_t));
    thread_data *data = (thread_data*) malloc(thread_count * sizeof (thread_data));

    block_count = FILE_SIZE / block_size;
    start_time = clock();

    //create dummy file
    file=fopen(filename, "w");
    fseek(file, FILE_SIZE-1, SEEK_SET);
    fputc('\n', file);
    fclose(file);
    
    //create threads
    for (n = 0; n < thread_count; n++) {
        data[n].operation_count = operations / thread_count;
        data[n].strategy = strategy;
        data[n].op_type = op_type;
        data[n].block_size = block_size;
        data[n].from = n * (block_count / thread_count);
        data[n].to = (n + 1)*(block_count / thread_count) - 1;
        pthread_create(&threads[n], NULL, perform, (void *) &data[n]);
    }

    //wait threads
    for (n = 0; n < thread_count; n++) {
        pthread_join(threads[n], NULL);
    }

    end_time = clock();
    

    total_seconds = (end_time - start_time) / ((double) CLOCKS_PER_SEC);
    //print in console
    printf("\nBlock size: %d, threads: %d, operation: %s, strategy: %s\n", block_size, thread_count, op_type_names[op_type],strategyNames[strategy]);
    printf("Total seconds: %f\n", total_seconds);
    printf("Mb: %f\n", (operations * (double) block_size / (double) 1048576));
    printf("Throughput: %f\n", (operations * (double) block_size / 1048576) / (double) total_seconds);
    *throughput = (operations * (double) block_size / 1048576) / (double) total_seconds;
   
}
