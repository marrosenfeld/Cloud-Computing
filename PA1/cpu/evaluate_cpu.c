/* 
 * File:   main.c
 * Author: mrosenfeld
 *
 * Created on January 26, 2016, 9:24 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

typedef enum {
    I, F
} op_type;
const char* op_type_names[] = {"Int", "FP"};
void measure(const int threads, const op_type op_type, double* value, int time);
void *perform(void *arg);//thread

//structure to pass all data to the thread
typedef struct thread_data {
    op_type op_type;
    double operations;
} thread_data;

int flag = 1;

int main(int argc, char** argv) {

    FILE *fp, *fp2;
    double *value;
    int threads[3] = {1, 2, 4};
    int i, k, it,iterations, seconds_per_iteration;
       
    //process arguments
    if(argc < 3) {
		printf("Usage: [no of iterations] [seconds per iteration]");
		exit(1);
	}
    iterations = atoi(argv[1]);
    seconds_per_iteration = atoi(argv[2]);
    
    value = (double*) malloc(sizeof (double));
    fp = fopen("result.csv", "w+"); //write the results in csv file
    fp2 = fopen("resultInTime.csv", "w+"); //write the results of the 10 minute run in csv file
    
    //header for console
    printf("%s\t%s\t%s\t\t%s\n", "Threads", "Op", "GOPS", "Seconds");
    
    //iterate through all possible combinations (total of 6 combinations)
    for (i = 0; i < (sizeof (threads) / sizeof (int)); i++) {
        for (k = I; k <= F; k++) {
            double total_th = 0.0;
            //loop the specified number of iterations
            for(it=0;it<iterations;it++){
                measure(threads[i], k, value, seconds_per_iteration);
                total_th += *value;
            }
            //write experiment result in file
            fprintf(fp, "%i\t%s\t%f\n", threads[i], op_type_names[k], total_th/(double)iterations);
        }
    }
    
    //experiment every 1 second
    for (k = I; k <= F; k++) {
        printf("\nStart 600 executions of %s operations\n", op_type_names[k]);
        for(i=0; i < 600; i++){
            measure(4, k, value, 1);
            fprintf(fp2, "%i, %f\n", i+1, *value);
        }
    }

    
    free(value);
    fclose(fp);
    fclose(fp2);
    return (EXIT_SUCCESS);
}

void *perform(void *arg) {
    thread_data *tdata = (thread_data *) arg;
    int i,r;
    float d = 0;
    double operations = 0;
    srand(time(NULL));
    r = rand();
    if(tdata->op_type == I)
    {
        //Perform Integer operations
        while(flag){
            i = i + r + 2;
            i = i + r - 5;
            i = i + r / 7;
            i = i + r * 9;
            operations += 9;
        }
    }
    else{
        //Perform Float Operations
        while(flag){
            d = d + 1.0 + (double)r;
            d = d * 5.0 + (double)r;
            d = d / 3.0 + (double)r;
            d = d - 4.0 + (double)r;
            
            operations += 8;
        }
    }
    
    //write back the number of operations performed
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
    
    //wait the specified time
    flag=1;
    sleep(time);
    flag = 0;
    
    //wait threads
    for (n = 0; n < thread_count; n++) {
        pthread_join(threads[n], NULL);
    }

    gettimeofday(&end_time, NULL);
    
    //collect the thread results
    operations = 0;
    for(n=0;n<thread_count;n++){
        operations += data[n].operations;
    }

    total_seconds =
         (double) (end_time.tv_usec - start_time.tv_usec) / 1000000 +
         (double) (end_time.tv_sec - start_time.tv_sec);
    
    //print in console    
    *value = ((operations / (double) total_seconds) / (double)1000000000);
    printf("%i\t%s\t%f\t%f\n", thread_count, op_type_names[op_type], *value, total_seconds);
    free(threads);
}
