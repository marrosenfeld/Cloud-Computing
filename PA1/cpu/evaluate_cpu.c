/* 
 * File:   main.c
 * Author: mrosenfeld
 *
 * Created on January 26, 2016, 9:24 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

typedef enum {
    I, F
} op_type;
const char* op_type_names[] = {"Integer", "Floating Point"};
void measure(const int threads, const op_type op_type, double* value, int time);
void *perform(void *arg);//thread

//structure to pass all data to the thread, to allow the thread perform the memcpy operations in different sections of the memory
typedef struct thread_data {
    op_type op_type;
    int seconds;
    double operations;
} thread_data;

int flag = 1;

int main(int argc, char** argv) {

    FILE *fp, *fp2;
    double *value;
    int threads[3] = {1, 2, 4};
    int i, k, it,iterations, seconds_per_iteration;
	setvbuf (stdout, NULL, _IONBF, 0);
	
    if(argc < 3) {
		printf("Usage: [no of iterations] [seconds per iteration]");
		exit(1);
	}

	iterations = atoi(argv[1]);
	seconds_per_iteration = atoi(argv[2]);
    printf("%i",seconds_per_iteration);
    value = (double*) malloc(sizeof (double));
    fp = fopen("result.csv", "w+"); //write the results in csv file
    fp2 = fopen("resultInTime.csv", "w+"); //write the results in csv file
    
    //iterate through all possible combinations (total of 8 combinations)
    for (i = 0; i < (sizeof (threads) / sizeof (int)); i++) {
        for (k = I; k <= F; k++) {
            double total_th = 0.0;
            for(it=0;it<iterations;it++){
                measure(threads[i], k, value, seconds_per_iteration);
                total_th += *value;
            }
            fprintf(fp, "%i, %s, %f\n", threads[i], op_type_names[k], total_th/(double)iterations);
        }
    }

    //experiment every 1 second
    for (k = I; k <= F; k++) {
        for(i=0; i < 60; i++){
            measure(4, k, value, 1);
            fprintf(fp2, "%i, %f\n", i, *value);
        }
    }
    
    free(value);
    fclose(fp);
    fclose(fp2);
    return (EXIT_SUCCESS);
}

void *perform(void *arg) {
    thread_data *tdata = (thread_data *) arg;
    int i;
    float d = 0;
    double operations = 0;
    if(tdata->op_type == I)
    {
        while(flag){
            i = i + 1 + 2 * 3 * 4 + 1 * 4 * 5 * 7 + 1 + 2 * 3 * 4 + 1 * 4 * 5 * 7;
            i = i + 1 + 2 * 3 * 4 + 1 * 4 * 5 * 7 + 1 + 2 * 3 * 4 + 1 * 4 * 5 * 7;
            i = i + 1 + 2 * 3 * 4 + 1 * 4 * 5 * 7 + 1 + 2 * 3 * 4 + 1 * 4 * 5 * 7;

            operations += 49;
        }
    }
    else{
        while(flag){
            d = d + 1.0 + 2.1 * 3.2 * 4.2 + 1.1 * 4.9 * 5.8 * 7.2 + 1.1 + 2.3 * 3.4 * 4.4 + 1.5 * 4.5 * 5.5 * 7.5;
            d = d + 1.0 + 2.1 * 3.2 * 4.2 + 1.1 * 4.9 * 5.8 * 7.2 + 1.1 + 2.3 * 3.4 * 4.4 + 1.5 * 4.5 * 5.5 * 7.5;
            d = d + 1.0 + 2.1 * 3.2 * 4.2 + 1.1 * 4.9 * 5.8 * 7.2 + 1.1 + 2.3 * 3.4 * 4.4 + 1.5 * 4.5 * 5.5 * 7.5;
            operations += 49;
        }
    }
    
    tdata -> operations = operations;
    pthread_exit(NULL);
}

void measure(const int thread_count, const op_type op_type, double* value, int time) {
    
     struct timeval start_time, end_time;//times
    int n;
    double total_seconds, operations;
    pthread_t *threads = (pthread_t*) malloc(thread_count * sizeof (pthread_t));
    thread_data *data = (thread_data*) malloc(thread_count * sizeof (thread_data));

    gettimeofday(&start_time, NULL);
    
    //create threads
    for (n = 0; n < thread_count; n++) {
        data[n].op_type = op_type;
        pthread_create(&threads[n], NULL, perform, (void *) &data[n]);
    }
    
    flag=1;
    sleep(time);
    flag = 0;
    
    //wait threads
    for (n = 0; n < thread_count; n++) {
        pthread_join(threads[n], NULL);
    }

    gettimeofday(&end_time, NULL);
    
    operations = 0;
    for(n=0;n<thread_count;n++){
        operations += data[n].operations;
    }

    total_seconds =
         (double) (end_time.tv_usec - start_time.tv_usec) / 1000000 +
         (double) (end_time.tv_sec - start_time.tv_sec);
    
    //print in console
    printf("\nThreads: %d, op_type: %s\n", thread_count, op_type_names[op_type]);
    printf("Total seconds: %f\n", total_seconds);
    printf("Operations: %f\n", operations);
    printf("OPS per second: %f\n", (operations ) / (double) total_seconds);
    printf("GOPS per second: %f\n", ((operations / (double) total_seconds) / (double)1000000000));
    *value = ((operations / (double) total_seconds) / (double)1000000000);
    free(threads);
}
