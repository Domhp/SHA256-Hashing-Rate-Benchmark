#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "sha256.h"
#include "sha256benchmark.h"

#define NUMBER_OF_HASHES 1000000
#define NUMBER_OF_THREADS 64
uint32_t nonce = 0;
pthread_mutex_t nonce_mutex = PTHREAD_MUTEX_INITIALIZER;
int running_all_threads = 1;
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;

void benchmark();
void* func(void* x);

int main(int argc, char *argv[]) {
    if( argc == 2 ) {
        printf("Benchmarking for %s seconds\n", argv[1]);
    }else {
        printf("One argument expected.\n");
        return 0;
    }
    benchmark(atoi(argv[1]));   
    return 0;
}
void* func(void* x){
    uint32_t temp; //Temp store for mutexed value
    /*Make the header to be "Minned" */
    HEADER header={.nonce= 0};
    /*Make header a byte array*/
    BYTE *data = (BYTE*)malloc(HEADER_SIZE);
    memcpy(data, (BYTE*)&header, HEADER_SIZE);
    int running = 1;    //running = true
    while(running){
        /*Get new nonce and Increment old nonce*/
        pthread_mutex_lock(&nonce_mutex);
        temp = nonce++;
        pthread_mutex_unlock(&nonce_mutex);
        /*Set new nonce to i*/
        memcpy(&data[HEADER_SIZE-4], &temp, sizeof(int32_t)); 
        /*Initialise values*/
        SHA256_CTX ctx;
        BYTE hash[SHA256_BLOCK_SIZE];

        sha256_init(&ctx);
        sha256_update(&ctx, data, HEADER_SIZE);
        sha256_final(&ctx,hash);
        pthread_mutex_lock(&running_mutex);
        running = running_all_threads;
        pthread_mutex_unlock(&running_mutex);
    }
    free(data);
    return NULL;
}
void benchmark(int seconds){
    pthread_t th[NUMBER_OF_THREADS];
    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        if(pthread_create(&th[i], NULL, &func, NULL) != 0){
            perror("Failed to create a Thread");
        }
    }
    //Sleep for specified period of time in args
    sleep(seconds);
    //Stop all threads running
    pthread_mutex_lock(&running_mutex);
    running_all_threads = 0; 
    pthread_mutex_unlock(&running_mutex);

    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join a Thread");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Number of Hashes: %d\nNumber of Threads: %d\n", nonce/*Changed in threads*/, NUMBER_OF_THREADS);
    printf("Hashes Per Second: %.2fMhash/s\n", (nonce/elapsed)/1000000);
    printf("Time taken = %.9f\n", elapsed);

    // const int size_hash = sizeof(hash);
    // for (int i = 0; i < size_hash; i++)
    // {
    //     printf("%02X", hash[i]);
    // }
    // printf("\n");

}
// for (int i = 0; i < HEADER_SIZE; i++)
// {
//     printf("%02X", data[i]);
// }
// printf("\n");