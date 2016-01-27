/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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
#define ARRAY_SIZE 80000000//150000000 //define an array size bigger than cache
      
typedef enum {SEQUENTIAL, RANDOM} strategy;
const char* strategyNames[] = {"Sequential", "Random"};
void measure(int block_size, const double operations, const int threads, const strategy strategy, double* throughput, double* latency);


int main(int argc, char** argv) {
    
    FILE *fp;
    double *throughput, *latency;
    int block_sizes[3] = {1,1024, 1048576};
    int threads[2] = {1,2};
    double operations[3] = { ARRAY_SIZE, 50000000,100000};
    int i,j,o,k;
    
    throughput = (double*)malloc(1*sizeof(double));
    latency = (double*)malloc(1*sizeof(double));
    fp = fopen("result.csv", "w+");
    o=0;
    for(i=0; i<(sizeof(block_sizes)/sizeof(int));i++){
        for(j=0; j<(sizeof(threads)/sizeof(int));j++){
            for(k=SEQUENTIAL; k<=RANDOM; k++){
                measure(block_sizes[i], operations[i], threads[j], k, throughput, latency);   
                fprintf(fp, "%i, %i, %i, %f, %f\n",block_sizes[i],threads[j],k,*throughput, *latency);
                o++;
            }
        }
    }
    
    fclose(fp);
    return (EXIT_SUCCESS);
}

void measure(const int block_size, const double operations, const int threads, const strategy strategy, double* throughput, double* latency){
    //source array to copy
    char *source_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    //target array to copy
    char *target_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    //times
    clock_t start_time, end_time;
    int i, block_count, block_number;
    double total_seconds;
    
    block_count = ARRAY_SIZE / block_size;
    //operations=ARRAY_SIZE/block_size;
    start_time = clock();
    srand(time(NULL));

    for(i = 0; i < operations; i++) 
    {
        if(strategy == SEQUENTIAL)
            block_number = i % block_count;
        else
            block_number = rand() % block_count;
        memcpy(target_array+(block_number*block_size),source_array+(block_number*block_size),block_size);   
    }
    
    end_time = clock();
    printf("\nBlock size: %d, threads: %d, strategy: %s\n", block_size, threads, strategyNames[strategy]);
    printf("Total seconds: %f\n", (end_time-start_time)/((double)CLOCKS_PER_SEC));
    printf("Mb: %f\n", (operations*(double)block_size/(double)1048576));
    
    total_seconds = (end_time-start_time)/((double)CLOCKS_PER_SEC);
    //total_seconds = (end_time-start_time)/100000;
    printf("Throughput: %f\n", (operations*(double)block_size/1048576) / (double)total_seconds);
    *throughput = (operations*(double)block_size/1048576) / (double)total_seconds;
    free(source_array);
    free(target_array);
}
