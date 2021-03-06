#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "sha256.h"
#include "sha256benchmark.h"
#include <math.h>

#define NUMBER_OF_HASHES 1000000
#define MAX_NUMBER_OF_THREADS 32
#define TIME_OF_EACH_BENCHMARK 900 //15 mins
uint32_t nonce = 0;
pthread_mutex_t nonce_mutex = PTHREAD_MUTEX_INITIALIZER;
int running_all_threads = 1;
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;
int hashes_found = 0;
pthread_mutex_t hashes_found_mutex = PTHREAD_MUTEX_INITIALIZER;

/*********************** FUNCTION DECLARATIONS **********************/
void benchmark(int threads);
void* func(void* x);
double difficulty(const unsigned bits);
int check_hash(BYTE hash[32]); //boolean
void print_hash(BYTE hash[32]);
/**
 * @brief Benchmarks different number of leading zeros
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]) {
    printf("Estimated time for whole benchmark: %d Minutes\n", TIME_OF_EACH_BENCHMARK * MAX_NUMBER_OF_THREADS / 60);
    for(int i = 1; i < MAX_NUMBER_OF_THREADS + 1; i++){
        benchmark(i);
    }
    return 0;
}
/**
 * @brief Thread to be created to "mine" header
 * 
 * @param x 
 * @return void* 
 */
void* func(void* x){
    uint32_t temp; //Temp store for mutexed value
    /*Make the header to be "Minned" */
    HEADER header={.nonce= 0, .time = (uint32_t)time(NULL)};
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
void benchmark(int threads){
    pthread_t * th; //Array for all threads
    th = (pthread_t*)malloc(sizeof(pthread_t) * threads);

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < threads; i++)
    {
        if(pthread_create(&th[i], NULL, &func, NULL) != 0){
            perror("Failed to create a Thread");
        }
    }
    //Sleep for specified period of time in args
    sleep(TIME_OF_EACH_BENCHMARK);
    //Stop all threads running
    pthread_mutex_lock(&running_mutex);
    running_all_threads = 0; 
    pthread_mutex_unlock(&running_mutex);

    for (int i = 0; i < threads; i++)
    {
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join a Thread");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Number of threads: %d\n", threads);
    printf("Number of Hashes: %d\nNumber of Threads: %d\n", nonce/*Changed in threads*/, threads);
    printf("Hashes Per Second: %.3fMhash/s\n", (nonce/elapsed)/1000000);
    printf("Time taken = %.9f\n", elapsed);
    printf("---------------------------------------------------------------------\n");
    /*Resets state */
    running_all_threads = 1;
    nonce = 0;
    hashes_found = 0;
    free(th);
}


/**
 * @brief prints 32 byte hash, doesnt check for length.
 * 
 * @param hash 
 */
void print_hash(BYTE hash[32]){
    for(int i = 0; i < 32; i++){
        printf("%02X:",hash[i]);
    }
    printf("\n");
}
