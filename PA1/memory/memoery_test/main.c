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

#define ARRAY_SIZE 80000000 //define an array size bigger than cache

typedef enum {
    SEQUENTIAL, RANDOM
} strategy;
const char* strategyNames[] = {"Sequential", "Random"};
void measure(int block_size, const double operations, const int threads, const strategy strategy, double* throughput, double* latency);
void *perform(void *arg);//thread

//structure to pass all data to the thread, to allow the thread perform the memcpy operations in different sections of the memory
typedef struct thread_data {
    int from, to, operation_count, block_size;
    strategy strategy;
    char* source_array, *target_array;
} thread_data;

int main(int argc, char** argv) {

    FILE *fp;
    double *throughput, *latency;
    int block_sizes[3] = {1, 1024, 1048576};
    int threads[2] = {1, 2};
    double operations[3] = {ARRAY_SIZE, 50000000, 100000}; //will perform different amount of operations for different block sizes
    int i, j, k;

    throughput = (double*) malloc(sizeof (double));
    latency = (double*) malloc(sizeof (double));
    fp = fopen("result.csv", "w+"); //write the results in csv file

    //iterate through all possible combinations (total of 12 combinations)
    for (i = 0; i < (sizeof (block_sizes) / sizeof (int)); i++) {
        for (j = 0; j < (sizeof (threads) / sizeof (int)); j++) {
            for (k = SEQUENTIAL; k <= RANDOM; k++) {
                measure(block_sizes[i], operations[i], threads[j], k, throughput, latency);
                fprintf(fp, "%i, %i, %s, %f, %f\n", block_sizes[i], threads[j], strategyNames[k], *throughput, *latency);

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
    char *target_array, *source_array;

    block_size = tdata->block_size;
    target_array = tdata->target_array;
    source_array = tdata->source_array;
    block_count = tdata->to - tdata->from + 1;
    srand(time(NULL));
    for (i = 0; i < tdata->operation_count; i++) {
        int r = rand();
        if (tdata->strategy == SEQUENTIAL)
            block_number = (i % block_count) + tdata->from;
        else
            block_number = (r % block_count) + tdata->from;
        memcpy(target_array + (block_number * block_size), source_array + (block_number * block_size), block_size);
    }
    pthread_exit(NULL);
}

void measure(const int block_size, const double operations, const int thread_count, const strategy strategy, double* throughput, double* latency) {
    
    char *source_array = (char*) malloc(ARRAY_SIZE * sizeof (char));//source array to copy
    char *target_array = (char*) malloc(ARRAY_SIZE * sizeof (char)); //target array to copy
    clock_t start_time, end_time;//times
    int block_count, n;
    double total_seconds;
    pthread_t *threads = (pthread_t*) malloc(thread_count * sizeof (pthread_t));
    thread_data *data = (thread_data*) malloc(thread_count * sizeof (thread_data));

    block_count = ARRAY_SIZE / block_size;
    start_time = clock();

    //create threads
    for (n = 0; n < thread_count; n++) {
        data[n].operation_count = operations / thread_count;
        data[n].strategy = strategy;
        data[n].source_array = source_array;
        data[n].target_array = target_array;
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
    printf("\nBlock size: %d, threads: %d, strategy: %s\n", block_size, thread_count, strategyNames[strategy]);
    printf("Total seconds: %f\n", total_seconds);
    printf("Mb: %f\n", (operations * (double) block_size / (double) 1048576));
    printf("Throughput: %f\n", (operations * (double) block_size / 1048576) / (double) total_seconds);
    *throughput = (operations * (double) block_size / 1048576) / (double) total_seconds;
    free(source_array);
    free(target_array);
}
