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
#define NUMBER_OF_THREADS 4
#define TIME_OF_EACH_BENCHMARK 2
uint32_t nonce = 0;
pthread_mutex_t nonce_mutex = PTHREAD_MUTEX_INITIALIZER;
int running_all_threads = 1;
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;
int hashes_found = 0;
pthread_mutex_t hashes_found_mutex = PTHREAD_MUTEX_INITIALIZER;
BYTE target[32] = {0}; //Used to check number of leading zeros
BYTE target_base_copy[32] = {0}; //Used to copy empty array

/*********************** FUNCTION DECLARATIONS **********************/
int benchmark(int leadingZeros);
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
    int max_leading_zeros = 8;
    for(int i = 1; i < max_leading_zeros+1; i++){
        int numHashes = benchmark(i);
        printf("STATS FOR %d LEADING ZERO(s):\nAverage time for leading zeros: %.10f\nNumber of hashes Found: %d\n",i,TIME_OF_EACH_BENCHMARK/(double)numHashes, numHashes);
        if(i != max_leading_zeros){
            printf("-----------------------------------------\n");
        }
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
        /*New code*/
        if(check_hash(hash) == 1){
            pthread_mutex_lock(&hashes_found_mutex);
            hashes_found++;
            pthread_mutex_unlock(&hashes_found_mutex);
        }
        /*Old code*/
        pthread_mutex_lock(&running_mutex);
        running = running_all_threads;
        pthread_mutex_unlock(&running_mutex);
    }
    free(data);
    return NULL;
}
/**
 * @brief benchmarks for certain leading zeros, called each new leading zeros
 * Used TIME_OF_EACH_BENCHMARK in seconds
 * @param leadingZeros number of leading zeros in desired hash.
 * @return int number of hashes with "leadingZeros" leading zeros.
 */
int benchmark(int leadingZeros){
    //Check odd leading, if, sett last to 0x0F
    if(leadingZeros % 2){
        target[(leadingZeros/2)] = 0x0f;
    }else{
        target[(leadingZeros/2)] = 0xff;
    }
    /*Below is copy of benchmark code, just sleeps for 10 mins*/
    pthread_t th[NUMBER_OF_THREADS];
    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        if(pthread_create(&th[i], NULL, &func, NULL) != 0){
            perror("Failed to create a Thread");
        }
    }
    sleep(TIME_OF_EACH_BENCHMARK);
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

    int result = hashes_found;
    /*Resets state */
    running_all_threads = 1;
    nonce = 0;
    hashes_found = 0;
    memcpy(target, target_base_copy, 32); //reset by just copying empty array for ease
    return result;
}
/**
 * @brief 
 * 
 * @param hash the 32 byte hash to be check
 * @return int 0(false) 1 (true)
 */
int check_hash(BYTE hash[32]){
    if(memcmp(hash, target, 32) <= 0){
        return 1;
    }return 0;
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
