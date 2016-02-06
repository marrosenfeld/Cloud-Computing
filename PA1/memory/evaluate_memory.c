/* 
 * File:   main.c
 * Author: mrosenfeld
 *
 * Created on January 26, 2016, 9:24 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#define ARRAY_SIZE 80000000 //define an array size bigger than cache

typedef enum {
    SEQUENTIAL, RANDOM
} strategy;
const char* strategyNames[] = {"Seq", "Random"};
void measure(int block_size, const int seconds, const int threads, const strategy strategy, double* throughput, double* latency);
void *perform(void *arg);//thread

int flag = 1;

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
    int i, j, k, it,iterations, seconds_per_iteration;

    if(argc < 3) {
            printf("Usage: [no of iterations] [seconds per iteration]\n");
            exit(1);
    }

    iterations = atoi(argv[1]);
    seconds_per_iteration = atoi(argv[2]);
    
    throughput = (double*) malloc(sizeof (double));
    latency = (double*) malloc(sizeof (double));
    fp = fopen("result.csv", "w+"); //write the results in csv file
    
    printf("%s\t%s\t%s\t%s\t\t%s\t\t%s\t%s\n", "Block", "Threads","Method", "MB", "Seconds", "Throughput", "Latency");
    //iterate through all possible combinations (total of 12 combinations)
    for (i = 0; i < (sizeof (block_sizes) / sizeof (int)); i++) {
        for (j = 0; j < (sizeof (threads) / sizeof (int)); j++) {
            for (k = SEQUENTIAL; k <= RANDOM; k++) {
                double total_th = 0.0;
                for(it=0;it<iterations;it++){
                    measure(block_sizes[i], seconds_per_iteration, threads[j], k, throughput, latency);
                    total_th += *throughput;
		}
                
                fprintf(fp, "%i\t%i\t%s\t%f\t%f\n", block_sizes[i], threads[j], strategyNames[k], total_th/(double)iterations, *latency);

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
    for (i = 0; flag; i++) {
        int r = rand();
        if (tdata->strategy == SEQUENTIAL)
            block_number = (i % block_count) + tdata->from;
        else
            block_number = (r % block_count) + tdata->from;
        memcpy(target_array + (block_number * block_size), source_array + (block_number * block_size), block_size);
    }
    tdata->operation_count = i;
    pthread_exit(NULL);
}

void measure(const int block_size, const int seconds, const int thread_count, const strategy strategy, double* throughput, double* latency) {
    
    char *source_array = (char*) malloc(ARRAY_SIZE * sizeof (char));//source array to copy
    char *target_array = (char*) malloc(ARRAY_SIZE * sizeof (char)); //target array to copy
    struct timeval start_time, end_time;//times
    int block_count, n, totalOperations;
    double total_seconds;
    pthread_t *threads = (pthread_t*) malloc(thread_count * sizeof (pthread_t));
    thread_data *data = (thread_data*) malloc(thread_count * sizeof (thread_data));

    block_count = ARRAY_SIZE / block_size;
    gettimeofday(&start_time, NULL);

    //create threads
    for (n = 0; n < thread_count; n++) {
        data[n].strategy = strategy;
        data[n].source_array = source_array;
        data[n].target_array = target_array;
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
    *throughput = (totalOperations * (double) block_size / 1048576) / (double) total_seconds;
    *latency = ( total_seconds / (double)totalOperations) * 1000;
    printf("%i\t%i\t%s\t%f\t%f\t%f\t%f\n", block_size, thread_count, strategyNames[strategy], (totalOperations * (double) block_size / 1048576),total_seconds,*throughput, *latency);
    free(source_array);
    free(target_array);
    free(data);
    free(threads);
}
