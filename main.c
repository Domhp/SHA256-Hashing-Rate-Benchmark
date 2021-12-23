#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "sha256.h"
#include "sha256benchmark.h"

#define NUMBER_OF_HASHES 1000000
#define NUMBER_OF_THREADS 20
uint32_t nonce = 0;
pthread_mutex_t nonce_mutex = PTHREAD_MUTEX_INITIALIZER;

void benchmark();
void* func(void* x);

int main() {
    benchmark();    
    return 0;
}
void* func(void* x){
    uint32_t temp; //Temp store for mutexed value
    /*Make the header to be "Minned" */
    HEADER header={.nonce= 0};
    /*Make header a byte array*/
    BYTE *data = (BYTE*)malloc(HEADER_SIZE);
    memcpy(data, (BYTE*)&header, HEADER_SIZE);

    for(uint32_t i = 0; i<NUMBER_OF_HASHES/NUMBER_OF_THREADS; i++){
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
    }
    free(data);
    return NULL;
}
void benchmark(){
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
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join a Thread");
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Number of Hashes: %d\nNumber of Threads: %d\n", NUMBER_OF_HASHES, NUMBER_OF_THREADS);
    printf("Hashes Per Second: %.2fMhash/s\n", (NUMBER_OF_HASHES/elapsed)/1000000);
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