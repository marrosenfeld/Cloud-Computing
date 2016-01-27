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
#include <sys/time.h>
#define ARRAY_SIZE 100000000 //define an array size bigger than cache
                  
void measure(int block_size, int iterations, int threads, double* throughput, double* latency);

int main(int argc, char** argv) {
    
    FILE *fp;
    fp = fopen("result.csv", "w+");
    double *throughput, *latency;
    int block_sizes[3] = {1,1000,1000000};
    int threads[2] = {1,2};
    int i,j;
    
    throughput = (double*)malloc(1*sizeof(double));
    latency = (double*)malloc(1*sizeof(double));
    
    for(i=0; i<(sizeof(block_sizes)/sizeof(int));i++){
        for(j=0; i<(sizeof(threads)/sizeof(int));j++){
            measure(block_sizes[i],10000, threads[j], throughput, latency);   
            fprintf(fp, "%i, %i, %f, %f\n",block_sizes[i],threads[j],*throughput, *latency);
        }
    }
    
    fclose(fp);
    return (EXIT_SUCCESS);
}

void measure(const int block_size, const int operations, const int threads, double* throughput, double* latency){
    //source array to copy
    void *source_array = (char*)malloc(block_size * operations * sizeof(char));
    //target array to copy
    void *target_array = (char*)malloc(block_size * operations * sizeof(char));
    clock_t* times = (clock_t*)malloc(operations * sizeof(clock_t));
    //times
    clock_t start_time, end_time, start_time_latency, end_time_latency;
    clock_t sum=0;
    int i;
    
    start_time = clock();
    
    for(i = 0; i < operations; i++) 
    {
        start_time_latency = clock();
        int block_number = (iterations/block_size) % (ARRAY_SIZE/block_size);
        memcpy(target_array+block_number,source_array+block_number,block_size);
        end_time_latency = clock();
       
        times[i] = end_time_latency - start_time_latency;
    }
    
    *throughput = (end_time-start_time)/(double)CLOCKS_PER_SEC;
    
    for(i = 0; i < iterations; i++){
        sum += times[i];
    }
    
    *latency = (sum / ((double)iterations)) /(double)CLOCKS_PER_SEC;
}